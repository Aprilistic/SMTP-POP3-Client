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
  
  //Socket *socket;
  //bool useTLS;

public:
  SMTP(std::string const &server, int port = 25, bool useTLS = true,
       std::string const &ID = "",
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