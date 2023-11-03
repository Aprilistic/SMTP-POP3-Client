#include "pop3/POP3.hpp"
#include "core/Socket.hpp"
#include "core/Email.hpp"

#include <iostream>
#include <sstream>
#include <string>

POP3::POP3(std::string const &server, int port, bool useTLS) {
  open(server, port, useTLS);
}

POP3::~POP3() { close(); }

void POP3::sendCommand(std::string const &command) {
  socket->write(command + "\r\n");
}

void POP3::getResponse(ServerResponse *response) {
  std::string buffer;
  socket->readLine(&buffer);

  if (buffer[0] == '+') {
    response->status = true;
    buffer.erase(0, 4); // Remove the "+OK "
  } else {
    response->status = false;
    buffer.erase(0, 5); // Remove the "-ERR "
  }

  response->statusMessage = buffer;
  response->data.clear();
}

void POP3::getMultilineData(ServerResponse *response) {
  std::string buffer;
  int bytesRead;

  while (true) {
    buffer.clear();

    bytesRead = socket->readLine(&buffer);

    if (buffer == "." || bytesRead == 0) {
      break;
    } else {
      if (buffer[0] == '.') /* Strip byte stuffed characters. */
      {
        buffer.erase(0, 1);
      }
      response->rawEmail += buffer;
      response->data.push_back(buffer);
    }
  }
}

void POP3::open(std::string const &server, int port, bool useTLS) {
  socket = new Socket(server, port, useTLS);

  ServerResponse welcomeMessage;

  getResponse(&welcomeMessage);

  if (!welcomeMessage.status) {
    throw ServerError("Conection refused", welcomeMessage.statusMessage);
  }
}

void POP3::close() {
  if (socket != NULL) {
    sendCommand("QUIT");
  }
  delete socket;
}

void POP3::authenticate(std::string const &username,
                        std::string const &password) {
  ServerResponse response;

  // Format the credentials: \0username\0password
  std::string credentials = "\0" + username + "\0" + password;

  // Base64 encode the credentials
  // std::string encodedCredentials = base64_encode(credentials);

  // Send the AUTH PLAIN command with the encoded credentials
  // sendCommand("AUTH PLAIN " + encodedCredentials);
  getResponse(&response);

  // Check the server's response
  if (!response.status) {
    throw ServerError("Authentication failed", response.statusMessage);
  }
}

void POP3::printMessageList() {
  ServerResponse response;

  sendCommand("LIST");

  getResponse(&response);
  if (!response.status) {
    throw ServerError("Unable to retrieve message list",
                      response.statusMessage);
  }

  getMultilineData(&response);

  if (response.data.size() == 0) {
    std::cout << "No messages available on the server." << std::endl;
  }

  int spacePosition = 0;
  for (std::list<std::string>::iterator line = response.data.begin();
       line != response.data.end(); line++) {
    spacePosition = line->find(' ');
    std::cout << line->substr(0, spacePosition) << std::endl;
  }
}

void POP3::printMessage(int messageId) {
  ServerResponse response;

  std::stringstream command;
  command << "RETR " << messageId;

  sendCommand(command.str());

  getResponse(&response);
  if (!response.status) {
    throw ServerError("Unable to retrieve requested message",
                      response.statusMessage);
  }

  getMultilineData(&response);

  for (std::list<std::string>::iterator line = response.data.begin();
       line != response.data.end(); line++) {
    std::cout << *line << std::endl;
  }
}

Email POP3::DownloadMail(int messageID){
  ServerResponse response;

  std::stringstream command;
  command << "RETR " << messageID;

  sendCommand(command.str());

  getResponse(&response);
  if (!response.status) {
    throw ServerError("Unable to retrieve requested message",
        response.statusMessage);
  }

  getMultilineData(&response);

  Email email(response.rawEmail);

  return email;
}
