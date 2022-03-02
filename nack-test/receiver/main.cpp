#include <iostream>
#include "Transport/boost-asio/udp_sender.h"


int main() {
  std::string ip("0.0.0.0");
  uint16_t port(8001);
  transportdemo::UDPSender sender(ip, port, 100);

  sender.run();

  std::cout << "sender run!" << std::endl;
  return 0;
}
