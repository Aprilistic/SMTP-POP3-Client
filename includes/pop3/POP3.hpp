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
  POP3(std::string const &server, int port = 995, bool useTLS = true,
       std::string const &ID = "",
       std::string const &Password = "");
  ~POP3();

  Email DownloadMessage(const int messageID);
  void PrintMessageList();
  void PrintMessage(int messageId);
  void DeleteMessage(int messageId);
  void ResetMailbox();

  /* Exceptions */
  class ServerError;

private:
  struct ServerResponse;
  void sendCommand(std::string const &command);
  void getResponse(ServerResponse *response);
  void getMultilineData(ServerResponse *response);

  void open(std::string const &server, int port, bool useTLS);
  void authenticate(std::string const &ID,
                    std::string const &Password);
  void close();

private:
  Socket *socket;
  bool useTLS;
};

struct POP3::ServerResponse {

  bool status; /*< It's true on +OK, false on -ERR */
  std::string statusMessage;
  std::list<std::string> data;
  std::string rawText;
};

class POP3::ServerError : public Error {
public:
  ServerError(std::string const &what, std::string const &serverStatus) {
    problem = what;
    reason = serverStatus;
  }
};

#endif
