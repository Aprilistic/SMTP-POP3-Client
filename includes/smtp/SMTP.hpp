#ifndef SMTP_HPP
#define SMTP_HPP

#include <list>
#include <string>
//#include <fstream>

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <netinet/in.h>

#include "core/Socket.hpp"

class Email;

class SMTP {
private:
  // 클라이언트로부터 넘겨받을 것
  /*
  const std::string authID = "AHNqdXRlc3RAbmF2ZXIuY29tAHNqdXRlc3RA";

  const std::string dnsAddress = "naver.com";
  const std::string smtpServerAddress = "smtp.naver.com";
  const int smtpPort = 25;

  struct sockaddr_in serv_addr;
  int status, valread, client_fd;
  SSL_CTX *ctx;
  SSL *ssl;

  std::ofstream report;
  char recvBuffer[0x200], sendBuffer[0x200];
  int recvBytes;
  */
  Socket *socket;
  bool useTLS;

public:
  SMTP(std::string const &server, int port = 25, bool useTLS = true,
       std::string const &ID = "",
       std::string const &Password = "");
  ~SMTP();

  void SendMail(Email email);

  class ServerError;

private:  
  struct ServerResponse;
  void sendCommand(std::string const &command);
  void getResponse(ServerResponse *response);

  void open(std::string const &server, int port, bool useTLS);
  void authenticate(std::string const &ID,
                    std::string const &Password);
  void close();
};

struct SMTP::ServerResponse {

  bool status; /*< It's true on 2XX, false on 3XX */
  std::string statusMessage;
  std::list<std::string> data;
};

class SMTP::ServerError : public Error {
public:
  ServerError(std::string const &what, std::string const &serverStatus) {
    problem = what;
    reason = serverStatus;
  }
};

#endif