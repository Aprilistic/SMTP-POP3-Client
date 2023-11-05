#include "core/Email.hpp"
#include "core/Base64.hpp"

#include <iostream>
#include <sstream>

std::string decodeMimeWord(const std::string &input) {
  if (input.substr(0, 10) == "=?UTF-8?B?" ||
      input.substr(0, 10) == "=?utf-8?B?") {
    std::string encoded = input.substr(
        10, input.length() - 12); // remove the MIME header and footer
    return base64_decode(encoded);
  }
  return input; // return the input if it's not encoded
}

Email::Email()
    : m_date(""), m_recvFrom(""), m_nickname(""), m_sendTo(""), m_title(""), m_body("") {}

Email::Email(std::list<std::string> &rawEmail) {
  m_body = "";
  bool isBase64Content = false;
  for (const auto &line : rawEmail) {
		std::cout << "line: " << line << std::endl;
    if (line.substr(0, 5) == "From:") {

      if (line[5] == ' ') {
        m_nickname = decodeMimeWord(line.substr(6));
        size_t start = line.find('<');
        size_t end = line.find('>');
        if (start != std::string::npos && end != std::string::npos &&
            start < end) {
          m_recvFrom = line.substr(start + 1, end - start - 1);
        } else{
          m_recvFrom = line.substr(6);
        }
      } else {
        m_recvFrom = line.substr(5);
      }
    } else if (line.substr(0, 3) == "To:") {
      if (line[3] == ' ') {
        m_sendTo = line.substr(4);
      } else {
        m_sendTo = line.substr(3);
      }
      // m_sendTo = line.substr(4);
    } else if (line.substr(0, 5) == "Date:") {
      if (line[5] == ' ') {
        m_date = line.substr(6);
      } else {
        m_date = line.substr(5);
      }
      // m_date = line.substr(6);
    } else if (line.substr(0, 8) == "Subject:") {
      if (line[8] == ' ') {
        m_title = decodeMimeWord(line.substr(9));
      } else {
        m_title = decodeMimeWord(line.substr(8));
      }
      // m_title = decodeMimeWord(line.substr(9));
    } else if (line.substr(0, 27) == "Content-Transfer-Encoding:") {
      isBase64Content = line.find("base64") != std::string::npos;
    } else if (isBase64Content) {
      // Assume that the body starts on the next line after
      // "Content-Transfer-Encoding: base64" and runs until the end of the list.
      m_body += base64_decode(line);
      //   isBase64Content = false; // Reset flag after decoding
    }
  }
}

void Email::PrintEmail() {
  std::cout << "Date: " << m_date << std::endl;
  std::cout << "From: " << m_nickname << " " << m_recvFrom << std::endl;
  std::cout << "To: " << m_sendTo << std::endl;
  std::cout << "Subject: " << m_title << std::endl;
  std::cout << "Body: " << m_body << std::endl;
}

// copy assignment operator
Email &Email::operator=(const Email &copy) {
  if (this != &copy) {
    this->m_date = copy.m_date;
    this->m_sendTo = copy.m_sendTo;
    this->m_nickname = copy.m_nickname;
    this->m_recvFrom = copy.m_recvFrom;
    this->m_title = copy.m_title;
    this->m_body = copy.m_body;
  }
  return *this;
}