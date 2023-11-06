# SMTP-POP3-Client

## Description

This is a simple email client compatible with the SMTP and POP3 standards, built using raw sockets. It employs SSL/TLS encryption by default and is designed to work with Naver's email servers.

## Features

- Send emails
- View inbox list
- Read emails
- Reply to emails

## Getting Started

This email client has been tested on Linux, Ubuntu, and macOS. To set up and use this client, follow the instructions below.

### Prerequisites

Before using the client, ensure that your system has the OpenSSL library installed and available in the PATH.

For Linux/Ubuntu:

```bash
sudo apt-get update
sudo apt-get install build-essential
sudo apt-get install libssl-dev
```
For macOS:

```bash
brew update
brew install openssl
echo 'export PATH="/opt/homebrew/Cellar/openssl@3/3.1.4"' >> ~/.bash_profile
source ~/.bash_profile
```

### Compilation
To compile the email client, perform the following steps:

1. Clone the repository:
2. Navigate to the root directory of the cloned repository
3. Compile the project using the `make` command
4. Run the program: `./EmailClient'

### Usage
To use the email client:

Login: Input your Naver account credentials that do not have two-factor authentication enabled.
Run the Program: Follow the on-screen instructions to manage and operate your emails.
Reconnection: If a timeout occurs and the connection is lost, you will be prompted to log in again.

![Alt text](/image/example.gif)

## License

This project is open-sourced under the MIT License. See the LICENSE file for more details.
