#ifndef POP3_HPP
#define POP3_HPP

#include <list>
#include <string>

#include "core/Base64.hpp"
#include "core/Error.hpp"
#include "core/Socket.hpp"

class Email;

class POP3 {
  Socket *socket;

public:
  POP3(std::string const &server, int port, bool useTLS = false);
  ~POP3();

  void authenticate(std::string const &username, std::string const &password);
  void printMessageList();
  void printMessage(int messageId);
  Email DownloadMail(int messageId);

  /* Exceptions */
  class ServerError;

private:
  struct ServerResponse;
  void sendCommand(std::string const &command);
  void getResponse(ServerResponse *response);
  void getMultilineData(ServerResponse *response);

  void open(std::string const &server, int port, bool useTLS);
  void close();
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
