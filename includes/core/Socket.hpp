#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "core/Error.hpp"

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <string>

class Socket {
  int socketFD;
  std::string address;
  std::string port;
  bool useTLS;

  SSL *ssl;
  SSL_CTX *ctx;

public:
  // Constructors and Destructor
  Socket(std::string const &inputAddress, int inputPort, bool useTLS = false);
  Socket(std::string const &inputAddress, std::string const &inputPort,
         bool useTLS = false);
  ~Socket();

  // Methods
  size_t read(char *buffer, size_t size);
  void write(std::string request);
  void readAll(std::string *response);
  size_t readLine(std::string *line);

  // Exceptions
  class ConnectionError;
  class IOError;

private:
  void open();
  void close();
  bool readCharacter(char *buffer);
  bool isReadyToRead();
  void initTLS();
  void cleanupTLS();
  void completeTLSHandshake();
};

class Socket::ConnectionError : public Error {
public:
  ConnectionError(std::string const &cause) {
    problem = "Unable to connect";
    reason = cause;
  }
};

class Socket::IOError : public Error {
public:
  IOError(std::string const &issue, std::string const &cause) {
    problem = issue;
    reason = cause;
  }
};

#endif
