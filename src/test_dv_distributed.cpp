#include <iostream>
#include <string>
#include "dv_distributed.hpp"

using namespace std;
using namespace rip;

int main(int argc, char const *argv[]) {
  if (argc > 2) {
    string ip          = string(argv[1]);
    Udp::port_t port   = stoi(string(argv[2]));
    string cmd, msg;

    Dv_distributed dv(host_t(ip, port));
    while (cin >> cmd) {
      if (cmd == "add") {
        cin >> ip >> port;
        dv.add_neighbor(host_t(ip, port));
      } else if (cmd == "send") {
        cin >> ip >> port >> msg;
        dv.route_message(host_t(ip, port), msg);
      } else if (cmd == "show") {
        const Dv::table_t & table = dv.get_table();
        for (auto it = table.begin(); it != table.end(); ++it) {
          cout << "|" << it->first.to_string()       << "\t|" <<
                         it->second.next.to_string() << "\t|" <<
                         it->second.cost             << "\t|" << endl;
        }
      }
    }
    // Dv::table_t table;
    // table.push_back(Dv::table_item(Dv::host_t("127.0.0.1", 1234), Dv::host_t("127.0.0.1", 2345), 1));
    // table.push_back(Dv::table_item(Dv::host_t("127.0.0.1", 1235), Dv::host_t("127.0.0.1", 2345), 2));
    // table.push_back(Dv::table_item(Dv::host_t("127.0.0.1", 1236), Dv::host_t("127.0.0.1", 2345), 3));
    // table.push_back(Dv::table_item(Dv::host_t("127.0.0.1", 1237), Dv::host_t("127.0.0.1", 2345), 4));
  }
  return 0;
}
