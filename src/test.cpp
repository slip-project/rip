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
    // Rip::table_t table;
    // table.push_back(Rip::table_item(Rip::host_t("127.0.0.1", 1234), Rip::host_t("127.0.0.1", 2345), 1));
    // table.push_back(Rip::table_item(Rip::host_t("127.0.0.1", 1235), Rip::host_t("127.0.0.1", 2345), 2));
    // table.push_back(Rip::table_item(Rip::host_t("127.0.0.1", 1236), Rip::host_t("127.0.0.1", 2345), 3));
    // table.push_back(Rip::table_item(Rip::host_t("127.0.0.1", 1237), Rip::host_t("127.0.0.1", 2345), 4));
    // Rip::table_t converted = Rip::parse_table(Rip::stringify_table(table));
    // for (auto it = converted.begin(); it != converted.end(); ++it) {
    //   cout << it->dest.to_string() << " " << it->next.to_string() << " " << it->cost << endl;
    // }
  }
  return 0;
}
