#ifndef MAILBOX_HPP
#define MAILBOX_HPP

#include "core/Email.hpp"

class MailBox{
public:
	static void SendMail(Email email);
    static Email RecvMail(int id);
	static bool DeleteMail(int id);
	static bool ForwardMail(int id, string &sendTo);
	static void ReplyMail(int id, string body);
	static void ListMailbox();		
};


#endif