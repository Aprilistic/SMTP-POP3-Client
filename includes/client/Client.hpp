#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <limits>

#include "core/Base64.hpp"
#include "core/Email.hpp"
#include "smtp/SMTP.hpp"
#include "pop3/POP3.hpp"
#include "client/Mailbox.hpp"

class Client
{
private:
    std::string ID;
    std::string AuthPlain;
    const std::string dnsAddress = "naver.com";
    Mailbox mailbox;
    SMTP smtp;
    POP3 pop3;

public:
    Client(const std::string &dns);

    bool Login();
    void Logout();
    Email EmailInput();
    void ShowOptions();
};

#endif