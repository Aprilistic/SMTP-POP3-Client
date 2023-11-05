
#include <cstdlib>
#include <string>

#include <fstream>
#include <iostream>

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <openssl/err.h>
#include <openssl/ssl.h>

#include "core/Socket.hpp"
#include "smtp/SMTP.hpp"

#include "core/Email.hpp" // 이메일 정보를 저장하는 클래스 헤더 파일

#include "core/Config.hpp"

SMTP::SMTP(std::string const &server, int port, std::string const &authplain)
{
    dnsAddress = __DOMAIN_NAME;
    smtpServerAddress = server;
    smtpPort = port;
    authID = authplain;
}// 생성자

SMTP::~SMTP(){
  CloseSMTP();
}

void SMTP::SMTPCycle(Email email) {
  // SMTP 클라이언트 주요 동작 함수

  ConnectSMTP();  // 서버 연결
  StartTlsSMTP(); // TLS 보안 연결
  AuthLogin();    // 이메일 아이디, 비밀번호 인증

  SendMail(email); // 이메일 전송

  CloseSMTP(); // 연결 종료
}

void SMTP::ConnectSMTP() {
  // SMTP 서버에 연결

  hostent *host;
  // 스트림 방식(TCP)의 소켓을 생성
  if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    throw std::runtime_error("Socket Failed");
  }

  // serv_addr 초기화
  memset(&serv_addr, 0x00, sizeof(serv_addr));

  // 주어진 호스트명(smtpServerAddress)에 대한 호스트 정보를 검색
  // 해당 호스트의 IP 주소 및 기타 네트워크 관련 정보를 반환
  host = gethostbyname(smtpServerAddress.c_str());
  if (host == NULL) {
    throw std::runtime_error("Host Not Found");
  }

  memcpy(&(serv_addr.sin_addr), host->h_addr, host->h_length); // IP 주소 복사
  serv_addr.sin_family = host->h_addrtype; // IPv4, IPv6 등 버전 지정
  serv_addr.sin_port =
      htons(smtpPort); // 포트번호를 빅 엔디안으로 인코딩하여 저장

  // 클라이언트 소켓 - 해당 SMTP 서버의 주소 및 포트와 연결 시도
  if ((status = connect(client_fd, (struct sockaddr *)&serv_addr,
                        sizeof(serv_addr))) < 0) {
    throw std::runtime_error("Connection Failed");
  }

  // 서버로부터 응답 수신
  // 220 smtp.naver.com ESMTP
  recvBytes = recv(client_fd, recvBuffer, sizeof(recvBuffer), 0);
  recvBuffer[recvBytes] = '\0';

  return;
}

void SMTP::StartTlsSMTP() {
  // TLS 연결 설정 및 SSL 핸드셰이크 수행

  // ehlo
  sprintf(sendBuffer, "ehlo %s\r\n", dnsAddress.c_str());
  send(client_fd, sendBuffer, (int)strlen(sendBuffer), 0);

  // 250-smtp.naver.com Pleased to meet you
  recvBytes = recv(client_fd, recvBuffer, sizeof(recvBuffer), 0);
  recvBuffer[recvBytes] = '\0';

  // STARTTLS 명령 전송
  sprintf(sendBuffer, "STARTTLS\r\n");
  send(client_fd, sendBuffer, (int)strlen(sendBuffer), 0);

  // 응답 읽기
  // 220 2.0.0 Ready to start TLS
  recvBytes = recv(client_fd, recvBuffer, sizeof(recvBuffer), 0);
  recvBuffer[recvBytes] = '\0';

  if (strstr(recvBuffer, "220") == NULL) {
    close(client_fd);
    throw std::runtime_error("STARTTLS not supported");
  }

  SSL_library_init(); // OpenSSL 라이브러리 초기화
  OpenSSL_add_all_algorithms(); // OpenSSL 라이브러리에서 사용할 수 있는 모든
                                // 암호화 알고리즘을 로드
  SSL_load_error_strings(); // OpenSSL 내부 오류 메시지를 로드, 오류 메시지를
                            // 인간이 이해하기 쉬운 형태로 출력할 수 있도록
                            // 도와줌

  // SSL 컨텍스트 생성
  ctx = SSL_CTX_new(SSLv23_client_method());
  if (ctx == NULL) {
    std::cerr << "SSL_CTX_new error" << std::endl;
  }

  // SSL 소켓 생성
  ssl = SSL_new(ctx);
  if (ssl == NULL) {
    std::cerr << "SSL_new error" << std::endl;
  }

  // TLS 연결 설정
  if (SSL_set_fd(ssl, client_fd) != 1) {
    std::cerr << "SSL_set_fd error" << std::endl;
  }

  // SSL 핸드셰이크 수행
  if (SSL_connect(ssl) != 1) {
    // SSL 핸드셰이크 실패 처리
    SSL_free(ssl);
    close(client_fd);
    throw std::runtime_error("SSL handshake failed");
  }
}

void SMTP::AuthLogin() {
  // SMTP 서버에 인증

  // ehlo
  sprintf(sendBuffer, "ehlo %s\r\n", dnsAddress.c_str());
  SSL_write(ssl, sendBuffer, (int)strlen(sendBuffer));

  // 250-smtp.naver.com Pleased to meet you
  memset(recvBuffer, 0, sizeof(recvBuffer));
  SSL_read(ssl, recvBuffer, sizeof(recvBuffer));

  // auth 로그인
  sprintf(sendBuffer, "AUTH PLAIN:\r\n");
  SSL_write(ssl, sendBuffer, (int)strlen(sendBuffer));

  // 334
  memset(recvBuffer, 0, sizeof(recvBuffer));
  SSL_read(ssl, recvBuffer, sizeof(recvBuffer));
  if (strstr(recvBuffer, "334") == NULL) {
    close(client_fd);
    throw std::runtime_error("AuthPlain not supported");
  }
  
  sprintf(sendBuffer, "%s\r\n", authID.c_str());
  SSL_write(ssl, sendBuffer, (int)strlen(sendBuffer));

  // 235 2.7.0 Accepted
  memset(recvBuffer, 0, sizeof(recvBuffer));
  SSL_read(ssl, recvBuffer, sizeof(recvBuffer));
  if (strstr(recvBuffer, "235") == NULL) {
    close(client_fd);
    throw std::runtime_error("Wrong ID or password");
  }
  return;
}

void SMTP::CloseSMTP() {
  // SMTP 연결 및 소켓 통신 종료
  if (ssl) {
    SSL_shutdown(ssl);
    SSL_free(ssl);
    ssl = nullptr;
  }
  if (ctx) {
    SSL_CTX_free(ctx);
    ctx = nullptr;
  }
  if (client_fd >= 0) {
    close(client_fd);
    client_fd = -1;
  }
}

void SMTP::SendMail(Email email) {
  // 메일 전송 요청 시작

  // MAIL FROM:<전송자 이메일>
  sprintf(sendBuffer, "MAIL FROM:<%s>\r\n", email.GetRecvFrom().c_str());
  SSL_write(ssl, sendBuffer, (int)strlen(sendBuffer));

  // 250 2.1.0 OK
  memset(recvBuffer, 0, sizeof(recvBuffer));
  SSL_read(ssl, recvBuffer, sizeof(recvBuffer));
  if (strstr(recvBuffer, "250") == NULL) {
    close(client_fd);
    throw std::runtime_error("Wrong Mail Sender Address");
  }

  // RCPT TO:<수신자 이메일>
  sprintf(sendBuffer, "RCPT TO:<%s>\r\n", email.GetSendTo().c_str());
  SSL_write(ssl, sendBuffer, (int)strlen(sendBuffer));

  // 250 2.1.5 OK
  memset(recvBuffer, 0, sizeof(recvBuffer));
  SSL_read(ssl, recvBuffer, sizeof(recvBuffer));
  if (strstr(recvBuffer, "250") == NULL) {
    close(client_fd);
    throw std::runtime_error("Wrong Mail Receiver Address");
  }

  // DATA
  sprintf(sendBuffer, "DATA\r\n");
  SSL_write(ssl, sendBuffer, (int)strlen(sendBuffer));

  // 354 Go ahead
  memset(recvBuffer, 0, sizeof(recvBuffer));
  SSL_read(ssl, recvBuffer, sizeof(recvBuffer));

  // 메일 본문 (To, From, Subject, Body)
  sprintf(sendBuffer, "To:%s\nFrom:%s\nSubject:%s\r\n\r\n%s\r\n.\r\n",
          email.GetSendTo().c_str(), email.GetRecvFrom().c_str(),
          email.GetTitle().c_str(), email.GetBody().c_str());
  SSL_write(ssl, sendBuffer, (int)strlen(sendBuffer));

  // 250 2.0.0 OK
  memset(recvBuffer, 0, sizeof(recvBuffer));
  SSL_read(ssl, recvBuffer, sizeof(recvBuffer));
  if (strstr(recvBuffer, "250") == NULL) {
    close(client_fd);
    throw std::runtime_error("Email delivery failure");
  }

  // quit
  sprintf(sendBuffer, "quit\r\n");
  SSL_write(ssl, sendBuffer, (int)strlen(sendBuffer));

  memset(recvBuffer, 0, sizeof(recvBuffer));
  SSL_read(ssl, recvBuffer, sizeof(recvBuffer));

  return;
}