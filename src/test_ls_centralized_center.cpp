#include <iostream>
#include "ls_centralized_center.hpp"

using namespace std;
using namespace rip;

int main(int argc, char const *argv[]) {

  if (argc > 2) {
    string ip          = string(argv[1]);
    Udp::port_t port   = stoi(string(argv[2]));
    string cmd, msg;

    Ls_centralized_center ls(host_t(ip, port));
    while (cin >> cmd) {
      if (cmd == "table") {
        const Ls::table_t & table = ls.get_table();
        for (auto it = table.begin(); it != table.end(); ++it) {
          cout << it->first.to_string() << " [";
          for (auto jt = it->second.begin(); jt != it->second.end(); ++jt) {
            cout << jt->to_string() << " ";
          }
          cout << "]" << endl;
        }
      }
    }
  }
  return 0;
}
