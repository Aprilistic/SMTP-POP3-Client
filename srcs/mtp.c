#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define MAX_SIZE 1024

char *base64(const char *input)
{
    if (input == NULL)
        return NULL;

    BIO *bmem = NULL;
    BIO *b64 = BIO_new(BIO_f_base64());
    BUF_MEM *bufferPtr = NULL;

    bmem = BIO_new(BIO_s_mem());
    if (bmem == NULL)
    {
        perror("Could not create a memory BIO");
        return NULL;
    }

    b64 = BIO_push(b64, bmem);

    // Disable newlines
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    if (BIO_write(b64, input, strlen(input)) <= 0 ||
        BIO_flush(b64) <= 0)
    {
        perror("Error in base64 encoding");
        return NULL;
    }

    // Get the length of the encoded data.
    size_t len = (size_t)(BIO_get_mem_data(bmem, &bufferPtr));

    char *buff = (char *)malloc(len + 1);
    if (!buff)
    {
        perror("Memory allocation failed");
        return NULL;
    }

    memcpy(buff, bufferPtr->data, len);
    buff[len] = '\0';

    BIO_free_all(b64);

    return buff;
}

void send_email(char *smtp_server, int port, char *from, char *to, char *password, char *subject, char *body, char **encoded_username_ptr, char **encoded_password_ptr)
{
    int socket_desc;
    struct sockaddr_in server;
    char message[MAX_SIZE], server_reply[MAX_SIZE];

    // Open the log file for writing.

    FILE *logfile = fopen("tsmtp.txt", "w");
    if (logfile == NULL)
    {
        perror("Could not open log file");
        return;
    }

    // Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        perror("Could not create socket");
        exit(EXIT_FAILURE);
    }
    server.sin_addr.s_addr = inet_addr(smtp_server);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    // Connect to remote server
    if (connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Connect error");
        printf("Errno: %d\n", errno);
        exit(1);
    }

    // Create a context for the TLS connection
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());

    if (ctx == NULL)
    {
        fprintf(stderr, "Error creating SSL context.\n");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Create an SSL connection and attach it to the socket
    SSL *ssl = SSL_new(ctx);
    if (ssl == NULL)
    {
        fprintf(stderr, "Error creating new SSL object.\n");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (!SSL_set_fd(ssl, socket_desc))
    {
        fprintf(stderr, "Error: Could not associate the socket descriptor with the SSL structure.");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Perform the handshake with the server
    if (SSL_connect(ssl) != 1)
    {
        fprintf(stderr, "Error: Could not establish SSL connection.\n");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Send HELO command to the SMTP Server
    sprintf(message, "HELO %s\r\n", smtp_server);
    if (SSL_write(ssl, message, strlen(message)) <= 0)
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
    fprintf(logfile, "Server reply for HELO: %s\n", server_reply);

    // Send AUTH LOGIN command
    strcpy(message, "AUTH LOGIN\r\n");
    if (SSL_write(ssl, message, strlen(message)) <= 0)
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
    fprintf(logfile, "Server reply for AUTH LOGIN: %s", server_reply);

    printf("%s", server_reply); // print server reply

    // Send username (Base64-encoded)
    *encoded_username_ptr = base64(from);
    sprintf(message, "%s\r\n", *encoded_username_ptr);
    if (SSL_write(ssl, message, strlen(message)) <= 0)
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
    fprintf(logfile, "Server reply for USERNAME: %s", server_reply);

    // Send password (Base64-encoded)
    *encoded_password_ptr = base64(password);
    sprintf(message, "%s\r\n", *encoded_password_ptr);
    if (SSL_write(ssl, message, strlen(message)) <= 0)
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
    fprintf(logfile, "Server reply for PASSWORD: %s\n", server_reply);

    // Send MAIL FROM command
    sprintf(message, "MAIL FROM:<%s>\r\n", from);
    if (SSL_write(ssl, message, strlen(message)) <= 0)
    {
        puts("Send failed");
        return;
    }
    fprintf(logfile, "Server reply after MAIL FROM:<%s>: %s\n", from, server_reply); // Log the Server Reply.

    // Receive server response for MAIL FROM command
    if (SSL_read(ssl, server_reply, MAX_SIZE) <= 0 || atoi(server_reply) < 200 || atoi(server_reply) > 299)
    {
        puts("recv failed or unexpected response");
        return;
    }

    // Send RCPT TO command
    sprintf(message, "RCPT TO:<%s>\r\n", to);
    if (SSL_write(ssl, message, strlen(message)) <= 0)
    {
        puts("Send failed");
        return;
    }

    // Receive server response for RCPT TO command
    if (SSL_read(ssl, server_reply, MAX_SIZE) <= 0 || atoi(server_reply) < 200 || atoi(server_reply) > 299)
    {
        puts("recv failed or unexpected response");
        return;
    }
    fprintf(logfile, "Server reply after RCPT TO:<%s>: %s\n", to, server_reply); // Log the Server Reply.

    // Send DATA command
    strcpy(message, "DATA\r\n");
    if (SSL_write(ssl, message, strlen(message)) <= 0)
    {
        puts("Send failed");
        return;
    }
    fprintf(logfile, "Response to DATA Command : %s\n", server_reply); // Log the Server Reply.

    // Receive server response for DATA command
    if (SSL_read(ssl, server_reply, MAX_SIZE) <= 0 || atoi(server_reply) != 354)
    {
        puts("recv failed or unexpected response");
        return;
    }

    // Send email headers and body
    sprintf(message, "Subject: %s\r\n\r\n%s\r\n.\r\n", subject, body);
    if (SSL_write(ssl, message, strlen(message)) <= 0)
    {
        puts("Send failed");
        return;
    }

    // Receive server response for email headers and body send operation.
    if (SSL_read(ssl, server_reply, MAX_SIZE) <= 0 || atoi(server_reply) < 200 || atoi(server_reply) > 299)
    {
        puts("recv failed or unexpected response");
        return;
    }
    fprintf(logfile, "Server reply after sending email headers and body: %s", server_reply);

    // Send QUIT command
    strcpy(message, "QUIT\r\n");
    if (SSL_write(ssl, message, strlen(message)) <= 0)
    {
        puts("Send failed");
        return;
    }

    // Receive server response for QUIT command
    if (SSL_read(ssl, server_reply, MAX_SIZE) <= 0)
    {
        puts("recv failed");
        return;
    }
    fprintf(logfile, "Server reply for QUIT: %s", server_reply);

    // Shutdown SSL connection
    int shutdown_result = SSL_shutdown(ssl);
    if (shutdown_result == 0)
    {
        // Incomplete shutdown, call SSL_shutdown again
        shutdown_result = SSL_shutdown(ssl);
    }
    if (shutdown_result < 0)
    {
        // handle error
    }
    if (ssl != NULL)
    {
        SSL_free(ssl);
    }

    if (ctx != NULL)
    {
        SSL_CTX_free(ctx);
    }

    close(socket_desc); // Close the socket descriptor.
    fclose(logfile);    // Close the log file.

    return;
}
void remove_newline(char *str)
{
    if ((strlen(str) > 0) && (str[strlen(str) - 1] == '\n'))
    {
        str[strlen(str) - 1] = '\0';
    }
}
int main()
{
    char from[100];
    char password[100];
    char to[100];
    char subject[200];
    char body[200];
    char *encoded_username = NULL;
    char *encoded_password = NULL;

    printf("Enter your email: ");
    fgets(from, sizeof(from), stdin);
    remove_newline(from);

    printf("Enter your password: ");
    fgets(password, sizeof(password), stdin);
    remove_newline(password);

    printf("Enter recipient's email: ");
    fgets(to, sizeof(to), stdin);
    remove_newline(to);

    printf("Enter subject of the email: ");

    // Remove the trailing newline character after fgets
    if ((strlen(subject) > 0) && (subject[strlen(subject) - 1] == '\n'))
        subject[strlen(subject) - 1] = '\0';
    fgets(subject, sizeof(subject), stdin);
    subject[strcspn(subject, "\n")] = '\0'; // Remove trailing newline
    remove_newline(subject);

    printf("Enter body of the email: ");
    if ((strlen(body) > 0) && (body[strlen(body) - 1] == '\n'))
        body[strlen(body) - 1] = '\0';
    fgets(body, sizeof(body), stdin);
    body[strcspn(body, "\n")] = '\0'; // Remove trailing newline
    remove_newline(body);

    send_email("smtp.naver.com", 465, from, to, password, subject, body, &encoded_username, &encoded_password);

    if (encoded_username != NULL)
    {
        free(encoded_username); // Don't forget to check for null before freeing.
    }
    if (encoded_password != NULL)
    {
        free(encoded_password); // Same here.
    }
    return 0;
}
