#include "client/MailBox.hpp"
#include "pop3/POP3.hpp"


void MailBox::SendMail(Email email){
	SMTP::SMTPCycle(email);
}

Email MailBox::RecvMail(int id){


}

bool MailBox::DeleteMail(int id){

}

//POP3에서 id번째 이메일을 읽어온 후 sendTo에게 그대로 전달
bool MailBox::ForwardMail(int id, string &sendTo){
	Email forwardEmail = POP3::DownloadMail(id);

	forwardEmail.SetRecvFrom(forwardEmail.GetSentTo());
	forwardEmail.SetSendTo(sendTo);
	forwardEmail.SetTitle("FW: " + forwardEmail.GetTitle);

	SMTP::SMTPCycle(forwardEmail);
}

//POP3에서 id번째 이메일을 읽어온 후 송/수신자 스왑, body를 채워 전송
void MailBox::ReplyMail(int id, string body){
	Email email = POP3::DownloadMail(id);
	Email replyEmail;

	replyEmail.SetSendTo(email.GetRecvFrom());
	replyEmail.SetRecvFrom(email.GetSendTo());
	replyEmail.SetTitle("RW: " + email.GetTitle());
	replyEmail.SetBody(body);

	SMTP::SMTPCycle(replyEmail);
}

void MailBox::ListMailbox(){

}		