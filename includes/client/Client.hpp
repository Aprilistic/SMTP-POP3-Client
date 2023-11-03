#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <limits>

#include "core/Base64.hpp"
#include "core/Config.hpp"
#include "core/Email.hpp"
#include "smtp/SMTP.hpp"
#include "pop3/POP3.hpp"
#include "MailBox.hpp"

class Client
{
private:
    std::string ID;
    std::string AuthPlain;
    const std::string dnsAddress;
    MailBox mailbox;

public:
    Client();
    bool Login();
    void Logout();
    Email EmailInput();
    void ShowOptions();
};

#endif