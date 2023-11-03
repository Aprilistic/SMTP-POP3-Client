#ifndef POP3SESSION_HPP
#define POP3SESSION_HPP

#include <list>
#include <string>

#include "core/base64.hpp"
#include "core/error.hpp"

class Socket;

class Pop3Session {
  std::unique_ptr<Socket> socket;

public:
  Pop3Session(std::string const &server, int port, bool useTLS = false);
  ~Pop3Session();

  void authenticate(std::string const &username, std::string const &password);
  void printMessageList();
  void printMessage(int messageId);

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

struct Pop3Session::ServerResponse {

  bool status; /*< It's true on +OK, false on -ERR */
  std::string statusMessage;
  std::list<std::string> data;
};

class Pop3Session::ServerError : public Error {
public:
  ServerError(std::string const &what, std::string const &serverStatus) {
    problem = what;
    reason = serverStatus;
  }
};

#endif
