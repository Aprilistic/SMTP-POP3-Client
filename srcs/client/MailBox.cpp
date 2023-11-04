#include <string>

#include "client/Client.hpp"
#include "client/MailBox.hpp"

MailBox::MailBox(std::string const &ID, std::string const &password,
                 std::string const &authplain)
    : m_ID(ID),
      smtp(__SMTP_SERVER_ADDRESS, __SMTP_DEFAULT_PORT, true, ID, authplain),
      pop3(__POP3_SERVER_ADDRESS, __POP3_DEFAULT_PORT, true, ID, password) {}

void MailBox::SendMail(Email email) { smtp.SendMail(email); }

Email MailBox::RecvMail(int id) { return (pop3.DownloadMessage(id)); }

bool MailBox::DeleteMail(int id) {
  // return (pop3.DeleteMail(id));
  return true;
}

// POP3에서 id번째 이메일을 읽어온 후 sendTo에게 그대로 전달
bool MailBox::ForwardMail(int id, std::string &sendTo) {
  Email forwardEmail = pop3.DownloadMessage(id);

  forwardEmail.SetRecvFrom(forwardEmail.GetSendTo());
  forwardEmail.SetSendTo(sendTo);

  std::string fw = "FW: ";
  fw += forwardEmail.GetTitle();
  forwardEmail.SetTitle(fw);

  smtp.SendMail(forwardEmail);
  return true;
}

// POP3에서 id번째 이메일을 읽어온 후 송/수신자 스왑, body를 채워 전송
void MailBox::ReplyMail(int id, std::string body) {
  Email email = pop3.DownloadMessage(id);
  Email replyEmail;

  replyEmail.SetSendTo(email.GetRecvFrom());
  replyEmail.SetRecvFrom(email.GetSendTo());

  std::string rw = "RW: ";
  rw += replyEmail.GetTitle();
  replyEmail.SetTitle(rw);
  replyEmail.SetBody(body);

  smtp.SendMail(replyEmail);
}

void MailBox::ListMailbox() { pop3.PrintMessageList(); }

void MailBox::ReadMail(int id) { pop3.PrintMessage(id); }