#include <iostream>
#include <string>
#include <queue>
#include <functional>
#include "ls_distributed.hpp"

using namespace std;
using namespace rip;

int main(int argc, char const *argv[]) {
  // Ls_distributed::table_t table, converted;
  // table[host_t("127.0.0.1", 1234)] = {
  //   host_t("127.0.0.1", 1235),
  //   host_t("127.0.0.1", 1237)
  // };
  // table[host_t("127.0.0.1", 1235)] = {
  //   host_t("127.0.0.1", 1234),
  //   host_t("127.0.0.1", 1236),
  //   host_t("127.0.0.1", 1237)
  // };
  // table[host_t("127.0.0.1", 1236)] = {
  //   host_t("127.0.0.1", 1235)
  // };
  // table[host_t("127.0.0.1", 1237)] = {
  //   host_t("127.0.0.1", 1234),
  //   host_t("127.0.0.1", 1235)
  // };

  // Ls_distributed ls("127.0.0.1", 1234);

  // ls.add_neibor(host_t("127.0.0.1", 1235));
  // ls.add_neibor(host_t("127.0.0.1", 1237));
  // ls.update_table(host_t("127.0.0.1", 1235), table);

  // Ls_distributed::router_t router = ls.get_router();

  // for (auto it = router.begin(); it != router.end(); ++it) {
  //   cout << it->first.to_string() << " => " << it->second.to_string() << endl;
  // }

  if (argc > 2) {
    string ip          = string(argv[1]);
    Udp::port_t port   = stoi(string(argv[2]));
    string cmd, msg;

    Ls_distributed dv(ip, port);
    while (cin >> cmd) {
      if (cmd == "add") {
        cin >> ip >> port;
        dv.add_neibor(host_t(ip, port));
      } else if (cmd == "send") {
        cin >> ip >> port >> msg;
        dv.route_message(host_t(ip, port), msg);
      } else if (cmd == "table") {
        const Ls::table_t & table = dv.get_table();
        for (auto it = table.begin(); it != table.end(); ++it) {
          cout << it->first.to_string() << " [";
          for (auto jt = it->second.begin(); jt != it->second.end(); ++jt) {
            cout << jt->to_string() << " ";
          }
          cout << "]" << endl;
        }
      } else if (cmd == "router") {
        const Ls::router_t & router = dv.get_router();
        for (auto it = router.begin(); it != router.end(); ++it) {
          cout << it->first.to_string() << " => " << it->second.to_string() << endl;
        }
      }
    }
  }
  return 0;
}
