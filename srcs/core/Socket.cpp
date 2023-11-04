#include "core/Socket.hpp"
#include "core/Config.hpp"
#include "core/Error.hpp"

#include <iostream>
#include <netdb.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <openssl/err.h>
#include <openssl/ssl.h>

Socket::Socket(const std::string &inputAddress, const std::string &inputPort,
               bool useTLS, PROTOCOL protocol)
    : useTLS(useTLS), protocol(protocol) {
  address = inputAddress;
  port = inputPort;
  socketFD = -1;
  if (useTLS) {
    initTLS();
  }
  
  open();
}

Socket::Socket(const std::string &inputAddress, int inputPort, bool useTLS,
               PROTOCOL protocol)
    : useTLS(useTLS), protocol(protocol) {
  address = inputAddress;
  socketFD = -1;
  std::stringstream portInString;
  portInString << inputPort;
  port = portInString.str();
  if (useTLS) {
    initTLS();
  }
  open();
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

  for (resultPointer = result; resultPointer != nullptr;
       resultPointer = resultPointer->ai_next) {
    socketFD = socket(resultPointer->ai_family, resultPointer->ai_socktype,
                      resultPointer->ai_protocol);
    if (socketFD == -1) {
      continue;
    }

    if (connect(socketFD, resultPointer->ai_addr, resultPointer->ai_addrlen) !=
        -1) {
      break;
    }

    ::close(socketFD);
    socketFD = -1;
  }

  freeaddrinfo(result);

  if (resultPointer == nullptr) {
    throw ConnectionError("Cannot establish connection to the server");
  }

  if (useTLS) {
    completeTLSHandshake();
  }
}

Socket::~Socket() {
  cleanupTLS();
  if (socketFD > 0) {
    close();
  }
}

void Socket::close() {
  ::shutdown(socketFD, SHUT_RDWR);
  ::close(socketFD);

  EVP_cleanup();
  socketFD = -1;
}

size_t Socket::read(char *buffer, size_t size) {
  if (!isReadyToRead()) {
    throw IOError("Receiving error",
                  "Server not responding (connection timed out).");
  }

  ssize_t bytesRead;
  if (useTLS) {
    bytesRead = SSL_read(ssl, buffer, size);
  } else {
    bytesRead = ::read(socketFD, buffer, size);
  }
  if (bytesRead < 0) {
    throw IOError("Receiving error", "Unable to read data from remote host");
  }

  return static_cast<size_t>(bytesRead);
}

void Socket::write(const std::string request) {
  ssize_t bytesWritten;
  if (useTLS) {
    bytesWritten = SSL_write(ssl, request.c_str(), request.length());
  } else {
    bytesWritten = ::write(socketFD, request.c_str(), request.length());
  }

  if (bytesWritten < 0) {
    throw IOError("Sending error", "Unable to send data to remote host");
  }
}

bool Socket::isReadyToRead() {
  if (useTLS && SSL_pending(ssl) > 0) {
    return true; // Data is available in OpenSSL's internal buffer
  }

  fd_set receiveFd;
  struct timeval timeout;
  FD_ZERO(&receiveFd);
  FD_SET(socketFD, &receiveFd);

  timeout.tv_sec = __SOCKET_READ_TIMEOUT;
  timeout.tv_usec = 0;

  return select(socketFD + 1, &receiveFd, nullptr, nullptr, &timeout) > 0;
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

void Socket::initTLS() {
  // SSL_library_init();	// OpenSSL 라이브러리 초기화
  // OpenSSL_add_all_algorithms();	//OpenSSL 라이브러리에서 사용할 수 있는 모든 암호화 알고리즘을 로드
  // SSL_load_error_strings();	// OpenSSL 내부 오류 메시지를 로드, 오류 메시지를 인간이 이해하기 쉬운 형태로 출력할 수 있도록 도와줌

  const SSL_METHOD *method = TLS_client_method();
  ctx = SSL_CTX_new(method);
  if (ctx == nullptr) {
    ERR_print_errors_fp(stderr);
    throw ConnectionError("Failed to create SSL context");
  }

  ssl = SSL_new(ctx);
  if (ssl == nullptr) {
    ERR_print_errors_fp(stderr);
    SSL_CTX_free(ctx);
    throw ConnectionError("Failed to create SSL structure");
  }
}

void Socket::completeTLSHandshake() {
  if (ssl == nullptr || ctx == nullptr) {
    throw ConnectionError("TLS is not initialized");
  }

  std::stringstream ss;
  if (protocol == PROTOCOL::SMTP) {
    ss << "ehlo "
       << "naver.com"
       << "\r\n";
    send(socketFD, ss.str().c_str(), (int)ss.str().length(), 0);

    ss.clear();
    ss.str("");
    char recvBuffer[1024];
    int recvBytes = recv(socketFD, recvBuffer, sizeof(recvBuffer), 0);
    recvBuffer[recvBytes] = '\0';

    ss << "STARTTLS\r\n";
    send(socketFD, ss.str().c_str(), (int)ss.str().length(), 0);

   

    recvBuffer[recvBytes] = '\0';

    std::cout << "recvBuffer: " << recvBuffer << std::endl;
    if (strstr(recvBuffer, "220") == NULL) {
      ::close(socketFD);
      throw std::runtime_error("'STARTTLS not supported'");
    }
  }

  SSL_library_init();	// OpenSSL 라이브러리 초기화
  OpenSSL_add_all_algorithms();	//OpenSSL 라이브러리에서 사용할 수 있는 모든 암호화 알고리즘을 로드
  SSL_load_error_strings();	// OpenSSL 내부 오류 메시지를 로드, 오류 메시지를 인간이 이해하기 쉬운 형태로 출력할 수 있도록 도와줌
  
  initTLS();

  SSL_set_fd(ssl, socketFD);
  if (SSL_connect(ssl) <= 0) {
    ERR_print_errors_fp(stderr);
    throw ConnectionError("Failed to establish TLS connection");
  }
}

void Socket::cleanupTLS() {
  if (ssl) {
    SSL_shutdown(ssl);
    SSL_free(ssl);
    ssl = nullptr;
  }

  if (ctx) {
    SSL_CTX_free(ctx);
    ctx = nullptr;
  }
}
