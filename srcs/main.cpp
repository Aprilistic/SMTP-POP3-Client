#include <iostream>
#include <string>

#include "client/Client.hpp"

int main() {
  Client client;

  std::cout << "Welcome to Simple Mail Client!" << std::endl;
  while (1) {
    try {
      client.ShowOptions();
    } catch (std::exception &e) {
      std::cout << e.what() << std::endl;
			client.Logout();
    }
  }
}
