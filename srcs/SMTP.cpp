// Client side C/C++ program to demonstrate Socket 
// programming 
#include <string>

#include <iostream>
#include <fstream>

#include <arpa/inet.h> 
#include <stdio.h> 
#include <string.h>

#include <sys/socket.h> 
#include <unistd.h> 
#include <netdb.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "Email.h" 
//#include "base64.hpp" 

//g++ SMTP.cpp -lssl -lcrypto



class SMTP {
private:

	//클라이언트로부터 넘겨받을 것
	const std::string authID = "AHNqdXRlc3RAbmF2ZXIuY29tAHNqdXRlc3RA";

	const std::string dnsAddress = "naver.com";
	const std::string smtpServerAddress = "smtp.naver.com";
	const int smtpPort = 25;

    struct sockaddr_in serv_addr; 
    int status, valread, client_fd;
	SSL_CTX *ctx;
	SSL *ssl;

	std::ofstream report;
	char recvBuffer[0x200], sendBuffer[0x200];
	int recvBytes;


public:

	SMTP();

	void SMTPCycle(Email);

    bool ConnectSMTP();
	bool StartTlsSMTP();
	bool AuthLogin();

	bool SendMail(Email);

	void CloseSMTP();
};




SMTP::SMTP() { }
void SMTP::SMTPCycle(Email email) {
	/** --------------------------------------------------------------------------------
	[                                  smtp 패킷전송                                   ]
	-------------------------------------------------------------------------------- **/



	/** --------------------------------------------------------------------------------
	[                                   패킷내용기록                                   ]
	-------------------------------------------------------------------------------- **/
	report.open("SMTP.txt");
	if (report.fail()) { printf("\nSMTP.txt failed.\n"); }
	if (!report.is_open()) { printf("\nSMTP.txt can not open the file.\n"); }
		
	ConnectSMTP();
	StartTlsSMTP();
	AuthLogin();

	SendMail(email);

	CloseSMTP();

	report.close();
}



bool SMTP::ConnectSMTP() {
	/** --------------------------------------------------------------------------------
	[                                    서버 접속                                     ]
	-------------------------------------------------------------------------------- **/
    		
	hostent* host;
	if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
		printf("\nSocket Failed \n");
		//throw std::runtime_error("socket");
	}


	memset(&serv_addr, 0x00, sizeof(serv_addr));

	host = gethostbyname(smtpServerAddress.c_str());
	if (host == NULL)
	{
		printf("\ngethostbyname\n");
	}
			
	memcpy(&(serv_addr.sin_addr), host->h_addr, host->h_length);
	serv_addr.sin_family = host->h_addrtype;
	serv_addr.sin_port = htons(smtpPort);


	if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) { 
		printf("\nConnection Failed \n");
	} 

	recvBytes = recv(client_fd, recvBuffer, sizeof(recvBuffer), 0);
	recvBuffer[recvBytes] = '\0';
	report << recvBuffer;

	return true;
}

bool SMTP::StartTlsSMTP(){
		
	/** --------------------------------------------------------------------------------
	[                                      ehlo                                        ]
	-------------------------------------------------------------------------------- **/
	sprintf(sendBuffer, "ehlo %s\r\n", dnsAddress.c_str());
	report << sendBuffer;
	send(client_fd, sendBuffer, (int)strlen(sendBuffer), 0);
	recvBytes = recv(client_fd, recvBuffer, sizeof(recvBuffer), 0);
	recvBuffer[recvBytes] = '\0';
	report << recvBuffer;

	/** --------------------------------------------------------------------------------
	[                                    startTLS                                      ]
	-------------------------------------------------------------------------------- **/
	// STARTTLS 명령 전송
	sprintf(sendBuffer, "STARTTLS\r\n");
	report << sendBuffer;
	send(client_fd, sendBuffer, (int)strlen(sendBuffer), 0);

	// 응답 읽기
	recvBytes = recv(client_fd, recvBuffer, sizeof(recvBuffer), 0);
	recvBuffer[recvBytes] = '\0';
	report << recvBuffer;

	if (strstr(recvBuffer, "220") == NULL) {
		close(client_fd);
		printf("\nSTARTTLS not supported\n");
		report.close();
		return false;
	}
		
	//int sockfd; // 소켓 디스크립터
		
	// OpenSSL 초기화
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();

	// SSL 컨텍스트 생성
	ctx = SSL_CTX_new(SSLv23_client_method());
	if (ctx == NULL) {
       	std::cerr << "SSL_CTX_new error" << std::endl;
	}
		
	// SSL 소켓 생성
	ssl = SSL_new(ctx);
	if (ssl == NULL) {
		std::cerr << "SSL_new error" << std::endl;
	}

	// TLS 연결 설정
	if (SSL_set_fd(ssl, client_fd) != 1) {
		std::cerr << "SSL_set_fd error" << std::endl;
	}


	// SSL 핸드셰이크 수행
	if (SSL_connect(ssl) != 1) {
		// SSL 핸드셰이크 실패 처리
		// 오류 메시지 출력 등
		SSL_free(ssl);
		close(client_fd);
		printf("\nSSL handshake failed\n");
		report.close();
		return false;
	}

	return true;

	// 이제 암호화된 통신을 사용하여 메일 전송을 계속할 수 있습니다.
}

bool SMTP::AuthLogin() {
	/** --------------------------------------------------------------------------------
	[                                      ehlo                                        ]
	-------------------------------------------------------------------------------- **/
	sprintf(sendBuffer, "ehlo %s\r\n", dnsAddress.c_str());
	report << sendBuffer << "\r\n";
	SSL_write(ssl, sendBuffer, (int)strlen(sendBuffer));
	SSL_read(ssl, recvBuffer, sizeof(recvBuffer));
	report << recvBuffer << "\r\n";

	/** --------------------------------------------------------------------------------
	[                                      auth                                        ]
	-------------------------------------------------------------------------------- **/
	sprintf(sendBuffer, "AUTH PLAIN:\r\n");
	report << sendBuffer << "\r\n";
	SSL_write(ssl, sendBuffer, (int)strlen(sendBuffer));

	SSL_read(ssl, recvBuffer, sizeof(recvBuffer));
	report << recvBuffer << "\r\n";
	sprintf(sendBuffer, "%s\r\n", authID.c_str());
	report << sendBuffer << "\r\n";
	SSL_write(ssl, sendBuffer, (int)strlen(sendBuffer));
	SSL_read(ssl, recvBuffer, sizeof(recvBuffer));
	report << recvBuffer << "\r\n";

	return true;
}

void SMTP::CloseSMTP() {
	SSL_shutdown(ssl);
	SSL_free(ssl);
	SSL_CTX_free(ctx);

	close(client_fd);
}


bool SMTP::SendMail(Email email){
	/** --------------------------------------------------------------------------------
	[                                      mail                                        ]
	-------------------------------------------------------------------------------- **/
	sprintf(sendBuffer, "MAIL FROM:<%s>\r\n", email.GetRecvFrom().c_str());
	report << sendBuffer << "\r\n";\
	SSL_write(ssl, sendBuffer, (int)strlen(sendBuffer));

	SSL_read(ssl, recvBuffer, sizeof(recvBuffer));
	report << recvBuffer << "\r\n";

	/** --------------------------------------------------------------------------------
	[                                      rcpt                                        ]
	-------------------------------------------------------------------------------- **/
	sprintf(sendBuffer, "RCPT TO:<%s>\r\n", email.GetSendTo().c_str());
	report << sendBuffer << "\r\n";
	SSL_write(ssl, sendBuffer, (int)strlen(sendBuffer));

	SSL_read(ssl, recvBuffer, sizeof(recvBuffer));
	report << recvBuffer << "\r\n";

	/** --------------------------------------------------------------------------------
	[                                      data                                        ]
	-------------------------------------------------------------------------------- **/
	sprintf(sendBuffer, "DATA\r\n");
	report << sendBuffer << "\r\n";
	SSL_write(ssl, sendBuffer, (int)strlen(sendBuffer));

	SSL_read(ssl, recvBuffer, sizeof(recvBuffer));
	report << recvBuffer << "\r\n";

	/** --------------------------------------------------------------------------------
	[                                     subject                                      ]
	-------------------------------------------------------------------------------- **/
	sprintf(sendBuffer, "To:%s\nFrom:%s\nSubject:%s\r\n\r\n%s\r\n.\r\n",
		email.GetSendTo().c_str(), email.GetRecvFrom().c_str(), email.GetTitle().c_str(), email.GetBody().c_str());
	report << sendBuffer << "\r\n";
	SSL_write(ssl, sendBuffer, (int)strlen(sendBuffer));

	SSL_read(ssl, recvBuffer, sizeof(recvBuffer));
	report << recvBuffer << "\r\n";

	/** --------------------------------------------------------------------------------
	[                                      quit                                        ]
	-------------------------------------------------------------------------------- **/
	sprintf(sendBuffer, "quit\r\n");
	report << sendBuffer << "\r\n";
	SSL_write(ssl, sendBuffer, (int)strlen(sendBuffer));

	SSL_read(ssl, recvBuffer, sizeof(recvBuffer));
	report << recvBuffer << "\r\n";

	return true;
}


int main(int argc, char const* argv[]) {
	try {
		SMTP smtpServer;
			
		Email email;
		std::string tmp;

		tmp = "kjunwoo23@gmail.com";
		email.SetSendTo(tmp);

		tmp = "sjutest@naver.com";
		email.SetRecvFrom(tmp);

		tmp = "test is test";
		email.SetTitle(tmp);

		tmp = "one\r\ntwo\r\nthree\r\n.\r\n";
		email.SetBody(tmp);

		smtpServer.SMTPCycle(email);
	}
	catch (const std::exception& e) {
		//MessageBoxA(0, e.what(), "", MB_OK);
	}
}
