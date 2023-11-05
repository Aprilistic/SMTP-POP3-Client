#include <iostream>
#include <signal.h>
#include <string>

#include "client/Client.hpp"

void handle_sigint(int sig) {
  std::cout << "\nCtrl + C를 눌러 프로그램이 종료됩니다.\n";
  logout
  exit(0);
}

int main() {
  Client client;

  if (signal(SIGINT, handle_sigint) == SIG_ERR) {
    std::cerr << "시그널 핸들러를 설정할 수 없습니다.\n";
    return 1;
  }

  std::cout << "Welcome to Simple Mail Client!\n" << std::endl;
  std::cout << "Ctrl + C를 눌러서 종료\n" << std::endl;
  while (1) {
    try {
      client.ShowOptions();
    } catch (std::exception &e) {
      std::cout << e.what() << std::endl;
      client.Logout();
    }
  }

  while(1);
}
