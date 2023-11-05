#ifndef SMTP_HPP
#define SMTP_HPP

#include <string>
#include <fstream>

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <netinet/in.h>

#include "core/Socket.hpp"

class Email;

class SMTP {
private:
  // 클라이언트로부터 넘겨받을 것
  std::string authID;

  std::string dnsAddress;
  std::string smtpServerAddress;
  int smtpPort;

  struct sockaddr_in serv_addr;
  int status, valread, client_fd;
  SSL_CTX *ctx;
  SSL *ssl;

  std::ofstream report;
  char recvBuffer[0x200], sendBuffer[0x200];
  int recvBytes;
  
  //Socket *socket;
  //bool useTLS;

public:
  SMTP(std::string const &server, int port = 25,
       std::string const &Password = "");
  ~SMTP();

  void SMTPCycle(Email email);

  void ConnectSMTP();
  void StartTlsSMTP();
  void AuthLogin();

  void SendMail(Email email);

  void CloseSMTP();
};

#endif