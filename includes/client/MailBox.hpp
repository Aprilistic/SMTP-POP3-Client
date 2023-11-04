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
	MailBox();
  
  SMTP smtp;
  POP3 pop3;
  void SendMail(Email email);
  Email RecvMail(int id);
  bool DeleteMail(int id);
  bool ForwardMail(int id, std::string &sendTo);
  void ReplyMail(int id, std::string body);
  void ListMailbox();

  void SetID(std::string &ID) { m_ID = ID; }
  void SetPassword(std::string &password) { m_password = password; }
  std::string &GetID() { return m_ID; }
  std::string &GetPassword() { return m_password; }
};

#endif