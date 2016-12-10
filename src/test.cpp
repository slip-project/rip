#include <iostream>
#include <string>
#include "udp.hpp"

using namespace std;
using namespace rip;

int main(int argc, char const *argv[]) {
  if (argc > 2) {
    Udp::port_t source_port = stoi(std::string(argv[1]));
    Udp::port_t dest_port   = stoi(std::string(argv[2]));
    Udp udp(source_port);
    udp.add_listener([](string dest_ip, Udp::port_t dest_port, string data)->void{
      cout << "receive from " << dest_ip << ":" << dest_port << " [" << data << "]" << endl;
    });
    string data;
    while (cin >> data) {
      udp.send("127.0.0.1", dest_port, data);
    }
  }
  return 0;
}
