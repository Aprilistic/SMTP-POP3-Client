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

#include "Email.h" // 이메일 정보를 저장하는 클래스 헤더 파일

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



SMTP::SMTP() { }	//생성자



void SMTP::SMTPCycle(Email email) {
    // SMTP 클라이언트 주요 동작 함수



	//.txt 파일에 패킷 통신 내용 기록	
	report.open("SMTP.txt");
	if (report.fail()) { printf("\nSMTP.txt failed.\n"); }
	if (!report.is_open()) { printf("\nSMTP.txt can not open the file.\n"); }

	ConnectSMTP();	//서버 연결
	StartTlsSMTP();	//TLS 보안 연결
	AuthLogin();	//이메일 아이디, 비밀번호 인증

	SendMail(email);	//이메일 전송

	CloseSMTP();	//연결 종료

	report.close();
}



bool SMTP::ConnectSMTP() {
	// SMTP 서버에 연결

	hostent* host;
	//스트림 방식(TCP)의 소켓을 생성
	if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
		throw std::runtime_error("Socket Failed");
	}
	//AF_INET은 인터넷 프로토콜을 의미
	//client_fd는 왜 int형인가?	-client_fd는 소켓 그 자체를 담는 변수가 아닌 파일 디스크립터로, 소켓의 연결을 나타내고 관리하는 변수이기 때문

	//serv_addr 초기화
	memset(&serv_addr, 0x00, sizeof(serv_addr));


	//주어진 호스트명(smtpServerAddress)에 대한 호스트 정보를 검색
	//해당 호스트의 IP 주소 및 기타 네트워크 관련 정보를 반환
	host = gethostbyname(smtpServerAddress.c_str());
	if (host == NULL)
	{
		throw std::runtime_error("Host Not Found");
	}
	
	memcpy(&(serv_addr.sin_addr), host->h_addr, host->h_length);	//IP 주소 복사
	serv_addr.sin_family = host->h_addrtype;	//IPv4, IPv6 등 버전 지정
	serv_addr.sin_port = htons(smtpPort);	//포트번호를 빅 엔디안으로 인코딩하여 저장


	//클라이언트 소켓 - 해당 SMTP 서버의 주소 및 포트와 연결 시도
	if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) { 
		throw std::runtime_error("Connection Failed");
	} 

	//서버로부터 응답 수신
	recvBytes = recv(client_fd, recvBuffer, sizeof(recvBuffer), 0);
	recvBuffer[recvBytes] = '\0';
	report << recvBuffer;

	return true;
}

bool SMTP::StartTlsSMTP(){
	// TLS 연결 설정 및 SSL 핸드셰이크 수행
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
	// SMTP 서버에 인증
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
	// SMTP 연결 및 소켓 통신 종료
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