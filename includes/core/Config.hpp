#ifndef CONFIG_HPP
#define CONFIG_HPP

#define __PROGRAM_NAME "EmailClient"

#define __DOMAIN_NAME "naver.com"

#define __SMTP_SERVER_ADDRESS "smtp.naver.com"
#define __SMTP_DEFAULT_PORT 465

#define __POP3_SERVER_ADDRESS "pop.naver.com"
#define __POP3_DEFAULT_PORT 995

/* Timeout for reading from socket. In case
   the server doesn't respond */
#define __SOCKET_READ_TIMEOUT 10 // seconds

#endif
