#ifndef MAILBOX_HPP
#define MAILBOX_HPP

#include "core/Email.hpp"
#include "pop3/POP3.hpp"
#include "smtp/SMTP.hpp"

class MailBox {
private:
  std::string m_ID;
  std::string m_password;

public:
	MailBox(std::string const &ID, std::string const &password);
  
  SMTP smtp;
  POP3 pop3;
  void SendMail(Email email);
  void ReadMail(int id);
  Email RecvMail(int id);
  bool DeleteMail(int id);
  bool ForwardMail(int id, std::string &sendTo);
  void ReplyMail(int id, std::string body);
  void ListMailbox();

  std::string &GetID() { return m_ID; }
  std::string &GetPassword() { return m_password; }
};

#endif