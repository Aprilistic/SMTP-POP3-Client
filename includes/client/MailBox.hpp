#ifndef MAILBOX_HPP
#define MAILBOX_HPP

#include "core/Email.hpp"
#include "pop3/POP3.hpp"
#include "smtp/SMTP.hpp"

class MailBox {
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
};

#endif