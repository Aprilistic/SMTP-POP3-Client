
#include <cstdlib>
#include <string>

#include <iostream>
#include <sstream>

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include "smtp/SMTP.hpp"
#include "core/Socket.hpp"

#include "core/Email.hpp"
#include "core/Config.hpp"


SMTP::SMTP(std::string const &server, int port, bool useTLS,
           std::string const &ID, std::string const &Password)
    : socket(nullptr), useTLS(useTLS) {
  open(server, port, useTLS);
  authenticate(ID, Password);
}// 생성자
SMTP::~SMTP() { close(); }


void SMTP::sendCommand(std::string const &command) {
  socket->write(command + "\r\n");
}

void SMTP::getResponse(ServerResponse *response) {
  std::string buffer;
  socket->readLine(&buffer);

  if (buffer[0] == '2' || buffer[0] == '3') {
    response->status = true;
    //buffer.erase(0, 4); // Remove the "2XX"
  } else {
    response->status = false;
    //buffer.erase(0, 5); // Remove the "3XX"
  }

  response->statusMessage = buffer;
  response->data.clear();
}

void SMTP::open(std::string const &server, int port, bool useTLS) {
  socket = new Socket(server, port, useTLS, PROTOCOL::SMTP);

  ServerResponse welcomeMessage;

  getResponse(&welcomeMessage);

  if (!welcomeMessage.status) {
    throw ServerError("Conection refused", welcomeMessage.statusMessage);
  }
}

void SMTP::close() {
  if (socket != NULL) {
    sendCommand("quit");
  }
  delete socket;
  socket = nullptr;
}

void SMTP::authenticate(std::string const &ID, std::string const &Password) {
  ServerResponse response;

  // ehlo
  std::stringstream ss;
  ss << "ehlo " << __SMTP_SERVER_ADDRESS;
  sendCommand(ss.str());
  getResponse(&response);
  ss.clear();
  ss.str("");

  // Check the server's response
  if (!response.status) {
    throw ServerError("auth-ehlo failed", response.statusMessage);
  }

  sendCommand("AUTH PLAIN:");
  getResponse(&response);

  if (!response.status) {
    throw ServerError("AuthPlain not supported", response.statusMessage);
  }

  sendCommand(Password);
  getResponse(&response);

  if (!response.status) {
    throw ServerError("Wrong ID or password", response.statusMessage);
  }

  std::cout << "SMTP Authentication successful" << std::endl;
}



void SMTP::SendMail(Email email){
  ServerResponse response;
  std::stringstream ss;

  // MAIL FROM:<전송자 이메일>
  ss << "MAIL FROM:<" << email.GetRecvFrom() << ">";
  sendCommand(ss.str());
  ss.str("");

  // 250 2.1.0 OK
  getResponse(&response);
  if (!response.status) {
    throw ServerError("Wrong Mail Sender Address", response.statusMessage);
  }

  // RCPT TO:<수신자 이메일>
  ss << "RCPT TO:<" << email.GetSendTo() << ">";
  sendCommand(ss.str());
  ss.str("");
  
  // 250 2.1.5 OK
  getResponse(&response);
  if (!response.status) {
    throw ServerError("Wrong Mail Receiver Address", response.statusMessage);
  }

  // DATA
  sendCommand("DATA");

  // 354 Go ahead
  getResponse(&response);
  if (!response.status) {
    throw ServerError("Wrong Command", response.statusMessage);
  }

  // 메일 본문 (To, From, Subject, Body)  
  ss << "To:" << email.GetSendTo() << "\n";
  ss << "From:" << email.GetRecvFrom() << "\n";
  ss << "Subject:" << email.GetTitle() << "\r\n\r\n";
  ss << "To:" << email.GetBody() << "\r\n.\r\n";
  sendCommand(ss.str());
  ss.str("");

  // 250 2.0.0 OK
  getResponse(&response);
  if (!response.status) {
    throw ServerError("Email Transmission failure", response.statusMessage);
  }
}