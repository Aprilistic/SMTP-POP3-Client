// Client side C/C++ program to demonstrate Socket 
// programming 
#include "SMTPClient.h" 
#define PORT 8080 

namespace Mawi1e {
	MySMTPClient::MySMTPClient() { }

	void MySMTPClient::SetServerPort(const int serverPort) { m_ServerPort = serverPort; }
	void MySMTPClient::SetSMTPServer(const std::string& smtpServer) { m_SMTPServer = smtpServer; }
	void MySMTPClient::SetDNSAddress(const std::string& dnsAddress) { m_DNSAddress = dnsAddress; }
	void MySMTPClient::SetEmailTo(const std::string& mailTo) { m_EmailTo = mailTo; }
	void MySMTPClient::SetEmailFrom(const std::string& mailFrom) { m_EmailFrom = mailFrom; }
	void MySMTPClient::SetSubject(const std::string& subject) { m_Subject = subject; }
	void MySMTPClient::SetMessage(const std::string& message) { m_Message = message; }

    struct sockaddr_in serv_addr; 
    int status, valread, client_fd; 
	void MySMTPClient::Transport() {
		/** --------------------------------------------------------------------------------
		[                                  smtp 패킷전송                                   ]
		-------------------------------------------------------------------------------- **/


		std::ofstream report;
		//SOCKET client_fd;
		char recvBuffer[0x200], sendBuffer[0x200];
		int recvBytes;

		/** --------------------------------------------------------------------------------
		[                                   패킷내용기록                                   ]
		-------------------------------------------------------------------------------- **/
		report.open("SMTP.txt");
		if (report.fail()) { printf("\nSMTP.txt failed.\n"); }
		if (!report.is_open()) { printf("\nSMTP.txt can not open the file.\n"); }

		//servSocket = Connect();
		Connect();

		/** --------------------------------------------------------------------------------
		[                                      ehlo                                        ]
		-------------------------------------------------------------------------------- **/
		recvBytes = recv(client_fd, recvBuffer, sizeof(recvBuffer), 0);
		recvBuffer[recvBytes] = '\0';
		report << recvBuffer;
		sprintf(sendBuffer, "ehlo %s\r\n", m_DNSAddress.c_str());
		report << sendBuffer;
		send(client_fd, sendBuffer, (int)strlen(sendBuffer), 0);

		/** --------------------------------------------------------------------------------
		[                                      mail                                        ]
		-------------------------------------------------------------------------------- **/
		recvBytes = recv(client_fd, recvBuffer, sizeof(recvBuffer), 0);
		recvBuffer[recvBytes] = '\0';
		report << recvBuffer;
		sprintf(sendBuffer, "mail from:<%s>\r\n", m_EmailFrom.c_str());
		report << sendBuffer;
		send(client_fd, sendBuffer, (int)strlen(sendBuffer), 0);

		/** --------------------------------------------------------------------------------
		[                                      rcpt                                        ]
		-------------------------------------------------------------------------------- **/
		recvBytes = recv(client_fd, recvBuffer, sizeof(recvBuffer), 0);
		recvBuffer[recvBytes] = '\0';
		report << recvBuffer;
		sprintf(sendBuffer, "rcpt to:<%s>\r\n", m_EmailTo.c_str());
		report << sendBuffer;
		send(client_fd, sendBuffer, (int)strlen(sendBuffer), 0);

		/** --------------------------------------------------------------------------------
		[                                      data                                        ]
		-------------------------------------------------------------------------------- **/
		recvBytes = recv(client_fd, recvBuffer, sizeof(recvBuffer), 0);
		recvBuffer[recvBytes] = '\0';
		report << recvBuffer;
		sprintf(sendBuffer, "data\r\n");
		report << sendBuffer;
		send(client_fd, sendBuffer, (int)strlen(sendBuffer), 0);

		/** --------------------------------------------------------------------------------
		[                                     subject                                      ]
		-------------------------------------------------------------------------------- **/
		recvBytes = recv(client_fd, recvBuffer, sizeof(recvBuffer), 0);
		recvBuffer[recvBytes] = '\0';
		report << recvBuffer;
		sprintf(sendBuffer, "To:%s\nFrom:%s\nSubject:%s\r\n\r\n%s\r\n.\r\n",
			m_EmailTo.c_str(), m_EmailFrom.c_str(), m_Subject.c_str(), m_Message.c_str());
		report << sendBuffer;
		send(client_fd, sendBuffer, (int)strlen(sendBuffer), 0);

		/** --------------------------------------------------------------------------------
		[                                      quit                                        ]
		-------------------------------------------------------------------------------- **/
		recvBytes = recv(client_fd, recvBuffer, sizeof(recvBuffer), 0);
		recvBuffer[recvBytes] = '\0';
		report << recvBuffer;
		sprintf(sendBuffer, "quit\r\n");
		report << sendBuffer;
		send(client_fd, sendBuffer, (int)strlen(sendBuffer), 0);


		recvBytes = recv(client_fd, recvBuffer, sizeof(recvBuffer), 0);
		recvBuffer[recvBytes] = '\0';
		report << recvBuffer;

		Close();

		report.close();
	}



	void MySMTPClient::Connect() {
		/** --------------------------------------------------------------------------------
		[                                    서버 접속                                     ]
		-------------------------------------------------------------------------------- **/
    		
	hostent* host;
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
			printf("\nSocket Failed \n");
		//throw std::runtime_error("socket");
    }


		memset(&serv_addr, 0x00, sizeof(serv_addr));

		host = gethostbyname(m_SMTPServer.c_str());
		if (host == NULL)
		{
			printf("\ngethostbyname\n");
		}
		
	memcpy(&(serv_addr.sin_addr), host->h_addr, host->h_length);
		serv_addr.sin_family = host->h_addrtype;
		serv_addr.sin_port = htons(m_ServerPort);

    //serv_addr.sin_family = AF_INET;
    //serv_addr.sin_family = host->h_addrtype;
    //serv_addr.sin_port = htons(m_ServerPort); 

    /*if (inet_pton(AF_INET, const_cast<char*>(m_SMTPServer.c_str()), &serv_addr.sin_addr) 
        <= 0) { 
        printf( 
            "\nInvalid address/ Address not supported \n"); 
    } */

    if ((status 
         = connect(client_fd, (struct sockaddr*)&serv_addr, 
                   sizeof(serv_addr))) 
        < 0) { 
			printf("\nConnection Failed \n");
			//throw std::runtime_error("\nConnection Failed \n");
    } 

	//return serv_addr;
	}

	void MySMTPClient::Close() {
    close(client_fd);
	}
}


int main(int argc, char const* argv[]) {
	try {
		Mawi1e::MySMTPClient smtpServer;

		const std::string dnsAddress("sju.ac.kr");
		const std::string emailFrom("kjunwoo23@sju.ac.kr");
		const std::string emailTo("kjunwoo23@naver.com");
		const std::string message("Hello There~ This is Junwoo Kim~!");
		//const std::string serverPort(25);
		const std::string smtpServerStr("smtp.sejong.ac.kr");
		const std::string subject("kjunwoo23's Test!");

		smtpServer.SetDNSAddress(dnsAddress);
		smtpServer.SetEmailFrom(emailFrom);
		smtpServer.SetEmailTo(emailTo);
		smtpServer.SetMessage(message);
		smtpServer.SetServerPort(25);
		smtpServer.SetSMTPServer(smtpServerStr);
		smtpServer.SetSubject(subject);

		smtpServer.Transport();
	}
	catch (const std::exception& e) {
		//MessageBoxA(0, e.what(), "", MB_OK);
	}
}