#ifndef MAILBOX_HPP
#define MAILBOX_HPP

#include "core/Email.hpp"
#include "pop3/POP3.hpp"
#include "smtp/SMTP.hpp"

class MailBox {
private:
  std::string m_ID;

public:
	MailBox(std::string const &ID, std::string const &password, std::string const &authplain);
  
  SMTP smtp;
  POP3 pop3;
  void SendMail(Email email);
  void ReadMail(int id);
  Email RecvMail(int id);
  void ReplyMail(int id, std::string body);
  void ListMailbox();

  std::string &GetID() { return m_ID; }
};

#endif