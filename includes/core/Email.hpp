#ifndef EMAIL_HPP
#define EMAIL_HPP

#include <string>
#include <list>


class Email {
private:
  std::string m_date;
  std::string m_sendTo;
  std::string m_recvFrom;
  std::string m_nickname;
  std::string m_title;
  std::string m_body;

public:
  Email();
  Email(std::list<std::string> &rawEmail);

  Email &operator=(const Email &copy);

  void PrintEmail();


  void SetDate(std::string &date) { m_date = date; }
  void SetSendTo(std::string &sendTo) { m_sendTo = sendTo; }
  void SetRecvFrom(std::string &recvFrom) { m_recvFrom = recvFrom; }
  void SetTitle(std::string &title) { m_title = title; }
  void SetBody(std::string &body) { m_body = body; }

  std::string &GetDate() { return m_date; }
  std::string &GetSendTo() { return m_sendTo; }
  std::string &GetRecvFrom() { return m_recvFrom; }
  std::string &GetTitle() { return m_title; }
  std::string &GetBody() { return m_body; }
};

#endif