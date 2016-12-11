#include <iostream>
#include <string>
#include "rip_distributed.hpp"

using namespace std;
using namespace rip;

int main(int argc, char const *argv[]) {
  if (argc > 2) {
    string ip          = string(argv[1]);
    Udp::port_t port   = stoi(string(argv[2]));
    Rip_distributed rip(ip, port);
    while (cin >> ip >> port) {
      rip.add_neibor(Rip::host_t(ip, port));
    }
  }
  return 0;
}
