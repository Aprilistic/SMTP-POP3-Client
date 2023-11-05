#include <iostream>
#include <signal.h>
#include <string>

#include "client/Client.hpp"

void leak(){
  system("leaks EmailClient");
}

int main() {
  atexit(leak);

  Client client;


  std::cout << "Welcome to Simple Mail Client!\n" << std::endl;
  while (1) {
    try {
      client.ShowOptions();
    } catch (std::exception &e) {
      std::cout << e.what() << std::endl;
      client.Logout();
      break;
    }
  }

}
