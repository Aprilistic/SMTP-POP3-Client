#include "core/socket.hpp"
#include "core/config.hpp"
#include "core/error.hpp"

#include <iostream>
#include <sstream>
#include <string>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <unistd.h>

Socket::Socket(std::string const &inputAddress, std::string const &inputPort,
               bool useTLS)
    : address(inputAddress), port(inputPort), socketFileDescriptor(-1),
      ssl(nullptr), ctx(nullptr), useTLS(useTLS) {
  open();
  if (useTLS) {
    initTLS();
  }
}

Socket::Socket(std::string const &inputAddress, int inputPort, bool useTLS)
    : address(inputAddress), socketFileDescriptor(-1), ssl(nullptr),
      ctx(nullptr), useTLS(useTLS) {
  std::stringstream portInString;
  portInString << inputPort;
  port = portInString.str();

  open();
  if (useTLS) {
    initTLS();
  }
}

void Socket::open() {
  struct addrinfo hints;
  struct addrinfo *result, *resultPointer;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  int getaddrinfoReturnCode =
      getaddrinfo(address.c_str(), port.c_str(), &hints, &result);
  if (getaddrinfoReturnCode != 0) {
    throw ConnectionError(gai_strerror(getaddrinfoReturnCode));
  }

  for (resultPointer = result; resultPointer != NULL;
       resultPointer = resultPointer->ai_next) {
    socketFileDescriptor =
        socket(resultPointer->ai_family, resultPointer->ai_socktype,
               resultPointer->ai_protocol);
    if (socketFileDescriptor == -1) {
      continue;
    }

    if (connect(socketFileDescriptor, resultPointer->ai_addr,
                resultPointer->ai_addrlen) != -1) {
      break;
    }

    ::close(socketFileDescriptor);
  }

  if (resultPointer == NULL) {
    throw ConnectionError("Cannot establish connection to the server");
  }

  ::freeaddrinfo(result);
}

Socket::~Socket() {
  if (socketFileDescriptor > 0) {
    close();
  }
}

void Socket::close() {
  ::shutdown(socketFileDescriptor, SHUT_RDWR);
  ::close(socketFileDescriptor);
}

size_t Socket::read(char *buffer, size_t size) {
  if (!isReadyToRead()) {
    throw IOError("Recieving error",
                  "Server not responding (connection timed out).");
  }

  ssize_t bytesRead = ::read(socketFileDescriptor, buffer, size);
  if (bytesRead < 0) {
    throw IOError("Recieving error", "Unable to resolve data from remote host");
  }

  return bytesRead;
}

void Socket::write(std::string request) {
  if (::write(socketFileDescriptor, request.c_str(), request.length()) < 0) {
    throw IOError("Sending error", "Unable to send data to remote host");
  }
}

void Socket::readAll(std::string *response) {
  char buffer;

  while (readCharacter(&buffer)) {
    *response += buffer;
  }
}

bool Socket::readCharacter(char *buffer) {
  if (read(buffer, 1) > 0) {
    return true;
  }
  return false;
}

size_t Socket::readLine(std::string *line) {
  char buffer[2] = "";
  *line = "";
  size_t bytesRead = 0;

  if (readCharacter(&(buffer[1]))) {
    do {
      *line += buffer[0]; // add char to string
      buffer[0] = buffer[1];
      bytesRead++;
    } while (readCharacter(&(buffer[1])) &&
             !(buffer[0] == '\r' && buffer[1] == '\n'));

    line->erase(0, 1);
  }

  return bytesRead;
}

bool Socket::isReadyToRead() {
  fd_set recieveFd;
  struct timeval timeout;
  int selectReturnValue;

  FD_ZERO(&recieveFd);
  FD_SET(socketFileDescriptor, &recieveFd);

  /* 30 seconds timeout */
  timeout.tv_sec = __SOCKET_READ_TIMEOUT;
  timeout.tv_usec = 0;

  selectReturnValue =
      select(socketFileDescriptor + 1, &recieveFd, NULL, NULL, &timeout);

  if (selectReturnValue > 0) {
    return true;
  }

  return false;
}

void Socket::initTLS() {
  SSL_library_init();
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();

  const SSL_METHOD *method = TLS_client_method();
  ctx = SSL_CTX_new(method);
  if (ctx == NULL) {
    ERR_print_errors_fp(stderr);
    throw ConnectionError("Failed to create SSL context");
  }

  ssl = SSL_new(ctx);
  if (ssl == NULL) {
    ERR_print_errors_fp(stderr);
    SSL_CTX_free(ctx);
    throw ConnectionError("Failed to create SSL structure");
  }

  SSL_set_fd(ssl, socketFileDescriptor);
  if (SSL_connect(ssl) <= 0) {
    ERR_print_errors_fp(stderr);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    throw ConnectionError("Failed to establish TLS connection");
  }
}

void Socket::cleanupTLS() {
  if (ssl) {
    SSL_shutdown(ssl);
    SSL_free(ssl);
    ssl = NULL;
  }
	
  if (ctx) {
    SSL_CTX_free(ctx);
    ctx = NULL;
  }
}