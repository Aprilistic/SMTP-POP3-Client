#ifndef POP3_HPP
#define POP3_HPP

#include <list>
#include <string>

#include "core/Base64.hpp"
#include "core/Error.hpp"
#include "core/Socket.hpp"

class Email;

class POP3 {
public:
  POP3(std::string const &server, int port, bool useTLS = false);
  ~POP3();

  Email DownloadMail(std::string const &encodedCredentials);

  /* Exceptions */
  class ServerError;

private:
  void authenticate(std::string const &encodedCredentials);
  void printMessageList();
  void printMessage(int messageId);

  struct ServerResponse;
  void sendCommand(std::string const &command);
  void getResponse(ServerResponse *response);
  void getMultilineData(ServerResponse *response);

  void open(std::string const &server, int port, bool useTLS);
  void close();

private:
  Socket *socket;
  const std::string server;
  const int port;
  bool useTLS;
};

struct POP3::ServerResponse {

  bool status; /*< It's true on +OK, false on -ERR */
  std::string statusMessage;
  std::list<std::string> data;
  std::string rawEmail;
};

class POP3::ServerError : public Error {
public:
  ServerError(std::string const &what, std::string const &serverStatus) {
    problem = what;
    reason = serverStatus;
  }
};

#endif
