#pragma once
//#pragma comment(lib, "ws2_32")
#pragma warning(disable:4996)

#include <iostream>
#include <fstream>

#include <arpa/inet.h> 
#include <stdio.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <unistd.h> 
#include <netdb.h>


namespace Mawi1e {
	class MySMTPClient {
	public:
		MySMTPClient();

		void SetServerPort(const int);
		void SetSMTPServer(const std::string&);
		void SetDNSAddress(const std::string&);
		void SetEmailTo(const std::string&);
		void SetEmailFrom(const std::string&);
		void SetSubject(const std::string&);
		void SetMessage(const std::string&);

		void Transport();

	private:
        void Connect();
		void Close();

	private:
		std::string m_SMTPServer, m_DNSAddress, m_EmailTo, m_EmailFrom, m_Subject, m_Message;
		int m_ServerPort;

	};
}