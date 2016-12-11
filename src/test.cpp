#include <iostream>
#include <string>
#include "rip_distributed.hpp"

using namespace std;
using namespace rip;

int main(int argc, char const *argv[]) {
  if (argc > 2) {
    string ip          = string(argv[1]);
    Udp::port_t port   = stoi(string(argv[2]));
    string cmd, msg;

    Rip_distributed rip(ip, port);
    while (cin >> cmd) {
      if (cmd == "add") {
        cin >> ip >> port;
        rip.add_neibor(Rip::host_t(ip, port));
      } else if (cmd == "send") {
        cin >> ip >> port >> msg;
        rip.route_message(Rip::host_t(ip, port), msg);
      } else if (cmd == "show") {
        const Rip::table_t & table = rip.get_table();
        for (auto it = table.begin(); it != table.end(); ++it) {
          cout << "|" << it->dest.to_string() << "\t|" <<
                         it->next.to_string() << "\t|" <<
                         it->cost             << "\t|" << endl;
        }
      }
    }
    // Rip::table_t table;
    // table.push_back(Rip::table_item(Rip::host_t("127.0.0.1", 1234), Rip::host_t("127.0.0.1", 2345), 1));
    // table.push_back(Rip::table_item(Rip::host_t("127.0.0.1", 1235), Rip::host_t("127.0.0.1", 2345), 2));
    // table.push_back(Rip::table_item(Rip::host_t("127.0.0.1", 1236), Rip::host_t("127.0.0.1", 2345), 3));
    // table.push_back(Rip::table_item(Rip::host_t("127.0.0.1", 1237), Rip::host_t("127.0.0.1", 2345), 4));
  }
  return 0;
}
