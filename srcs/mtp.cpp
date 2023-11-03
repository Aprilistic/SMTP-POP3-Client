#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>

using namespace std;

#define MAX_SIZE 1024

string base64_encode(const string &s)
{
    using namespace boost::archive::iterators;
    typedef base64_from_binary<transform_width<string::const_iterator, 6, 8>> base64_enc;
    auto ptr = s.c_str();
    auto len = s.size();
    return string(base64_enc(ptr), base64_enc(ptr + len));
}

void send_email(const string &smtp_server, int port, const string &from,
                const string &to, const string &password,
                const string &subject, const string &body)
{
    int socket_desc;
    struct sockaddr_in server;
    char server_reply[MAX_SIZE];
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
    hints.ai_socktype = SOCK_STREAM;

    // Open the log file for writing.
    ofstream logfile("tsmtp.txt");
    if (!logfile.is_open())
    {
        perror("Could not open log file");
        return;
    }

    // get ready to connect
    int status = getaddrinfo(smtp_server.c_str(), "465", &hints, &res);
    if (status != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    // Create socket
    socket_desc = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (socket_desc == -1)
    {
        perror("Could not create socket");
        exit(EXIT_FAILURE);
    }

    // Connect to remote server
    if (connect(socket_desc, res->ai_addr, res->ai_addrlen) < 0)
    {
        perror("Connect error");
        printf("Errno: %d\n", errno);
        exit(1);
    }

    freeaddrinfo(res); // free the linked list of address info

    // Create a context for the TLS connection
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());

    if (ctx == NULL)
    {
        cerr << "Error creating SSL context." << endl;
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Create an SSL connection and attach it to the socket
    SSL *ssl = SSL_new(ctx);

    if (ssl == NULL)
    {
        cerr << "Error creating new SSL object." << endl;
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (!SSL_set_fd(ssl, socket_desc))
    {
        cerr << "Error: Could not associate the socket descriptor with the SSL structure." << endl;
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Perform the handshake with the server
    int ret = SSL_connect(ssl);

    if (ret != 1)
    {
        int err = SSL_get_error(ssl, ret); // Get more detailed error information

        switch (err)
        {
        case SSL_ERROR_NONE:
            break;
        case SSL_ERROR_ZERO_RETURN:
            break;
        case SSL_ERROR_WANT_READ:
        case SSL_ERROR_WANT_WRITE:
            break;
        default:
            char buf[256];
            ERR_error_string_n(err, buf, sizeof(buf));
            printf("SSL connect failed: %s\n", buf);
            break;
        }
        exit(EXIT_FAILURE);
    }
    // Send HELO command to the SMTP Server
    string message = "HELO " + smtp_server + "\r\n";
    if (SSL_write(ssl, message.c_str(), message.length()) <= 0)
    {
        puts("Send failed");
        return;
    }

    // Receive server response for HELO
    if (SSL_read(ssl, server_reply, MAX_SIZE) <= 0)
    {
        puts("recv failed");
        return;
    }

    logfile << "Server reply for HELO: " << server_reply << endl;

    // Send AUTH LOGIN command
    message = "AUTH LOGIN\r\n";
    if (SSL_write(ssl, message.c_str(), message.length()) <= 0)
    {
        puts("Send failed");
        return;
    }

    // Receive server response for AUTH LOGIN
    if (SSL_read(ssl, server_reply, MAX_SIZE) <= 0)
    {
        puts("recv failed");
        return;
    }

    logfile << "Server reply for AUTH LOGIN: " << server_reply;

    printf("%s", server_reply); // print server reply

    // Send username (Base64-encoded)
    string encoded_username = base64_encode(from);
    message = encoded_username + "\r\n";

    if (SSL_write(ssl, message.c_str(), message.length()) <= 0)
    {
        puts("Send failed");
        return;
    }

    // Receive server response for username
    if (SSL_read(ssl, server_reply, MAX_SIZE) <= 0)
    {
        puts("recv failed");
        return;
    }

    logfile << "Server reply for USERNAME: " << server_reply;

    // Send password (Base64-encoded)
    string encoded_password = base64_encode(password);
    message = encoded_password + "\r\n";

    if (SSL_write(ssl, message.c_str(), message.length()) <= 0)
    {
        puts("Send failed");
        return;
    }

    // Receive server response for password
    if (SSL_read(ssl, server_reply, MAX_SIZE) <= 0)
    {
        puts("recv failed");
        return;
    }

    logfile << "Server reply for PASSWORD: " << server_reply;

    // Send MAIL FROM command
    message = "MAIL FROM:<" + from + ">\r\n";

    if (SSL_write(ssl, message.c_str(), message.length()) <= 0)
    {
        puts("Send failed");
        return;
    }

    // Receive server response for MAIL FROM
    if (SSL_read(ssl, server_reply, MAX_SIZE) <= 0)
    {
        puts("recv failed");
        return;
    }

    logfile << "Server reply for MAIL FROM: " << server_reply;

    // Send RCPT TO command
    message = "RCPT TO:<" + to + ">\r\n";

    if (SSL_write(ssl, message.c_str(), message.length()) <= 0)
    {
        puts("Send failed");
        return;
    }

    // Receive server response for RCPT TO
    if (SSL_read(ssl, server_reply, MAX_SIZE) <= 0)
    {
        puts("recv failed");
        return;
    }

    logfile << "Server reply for RCPT TO: " << server_reply;

    // Send DATA command
    message = "DATA\r\n";
    if (SSL_write(ssl, message.c_str(), message.length()) <= 0)
    {
        puts("Send failed");
        return;
    }

    // Receive server response for DATA
    if (SSL_read(ssl, server_reply, MAX_SIZE) <= 0)
    {
        puts("recv failed");
        return;
    }

    logfile << "Server reply for DATA: " << server_reply;

    // Now send email headers and body
    message = "From: <" + from + ">\r\n" +
              "To: <" + to + ">\r\n" +
              "Subject: " + subject + "\r\n\r\n" +
              body + "\r\n.\r\n";

    if (SSL_write(ssl, message.c_str(), message.length()) <= 0)
    {
        puts("Send failed");
        return;
    }

    // Receive server response for email data sent.
    if (SSL_read(ssl, server_reply, MAX_SIZE) <= 0)
    {
        puts("recv failed");
        return;
    }

    logfile << "Server reply after sending email data: " << server_reply;

    // Send QUIT command
    message = "QUIT\r\n";

    if (SSL_write(ssl, message.c_str(), message.length()) <= 0)
    {
        puts("Send failed");
        return;
    }

    // Receive server response for QUIT
    if (SSL_read(ssl, server_reply, MAX_SIZE) <= 0)
    {
        puts("recv failed");
        return;
    }

    logfile << "Server reply for QUIT: " << server_reply;

    // Close the connection and cleanup
    SSL_shutdown(ssl);
    SSL_free(ssl);

    close(socket_desc);

    logfile.close();
}

int main()
{
    string from, password, to, subject, body;

    cout << "Enter your email: ";
    getline(cin, from);

    cout << "Enter your password: ";
    getline(cin, password);

    cout << "Enter recipient's email: ";
    getline(cin, to);

    cout << "Enter subject of the email: ";
    getline(cin, subject);

    cout << "Enter body of the email:";
    getline(cin, body);

    send_email("smtp.naver.com", 465, from, to, password, subject, body);

    return 0;
}
