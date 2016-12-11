#include <iostream>
#include "ls_centralized_client.hpp"

using namespace std;
using namespace rip;

int main(int argc, char const *argv[]) {

  if (argc > 4) {
    string local_ip         = string(argv[1]);
    Udp::port_t local_port  = stoi(string(argv[2]));
    string center_ip        = string(argv[3]);
    Udp::port_t center_port = stoi(string(argv[4]));
    string cmd, msg;

    Ls_centralized_client ls(host_t(local_ip, local_port), host_t(center_ip, center_port));
    string ip;
    Udp::port_t port;
    while (cin >> cmd) {
      if (cmd == "add") {
        cin >> ip >> port;
        ls.add_neibor(host_t(ip, port));
      } else if (cmd == "send") {
        cin >> ip >> port >> msg;
        ls.route_message(host_t(ip, port), msg);
      } else if (cmd == "router") {
        const Ls::router_t & router = ls.get_router();
        for (auto it = router.begin(); it != router.end(); ++it) {
          cout << it->first.to_string() << " => " << it->second.to_string() << endl;
        }
      }
    }
  }
  return 0;
}
