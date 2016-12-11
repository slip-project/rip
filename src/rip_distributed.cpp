#include "rip_distributed.hpp"
#include <iostream>

#define DEBUG

#ifdef DEBUG
static void log(std::string message) {
  std::cout << "[rip dist]\t" << message << std::endl;
}
#endif

  rip::Rip_distributed::Rip_distributed(std::string ip, Udp::port_t port) : Rip(ip, port) {}
  rip::Rip_distributed::~Rip_distributed() {}

  void rip::Rip_distributed::sync() {
    for (auto it = _neibors.begin(); it != _neibors.end(); ++it) {

      #ifdef DEBUG
      log("sync with " + it->dest.to_string());
      #endif

      send_table(it->dest);
    }
  }

  rip::Rip_distributed::neibor_ptr rip::Rip_distributed::add_neibor(host_t host) {
    auto neibor_p = Rip::add_neibor(host);
    update_neibor_timer(neibor_p);
    _table.push_back(table_item(host, host, 1));

    #ifdef BUDEG
    log("neibor + " + neibor_p->dest.to_string() + " added");
    #endif

    return neibor_p;
  }

  void rip::Rip_distributed::remove_neibor(neibor_ptr neibor_p) {
    for (auto it = _table.begin(); it != _table.end();) {
      if (it->dest == neibor_p->dest || it->next == neibor_p->dest) {
        it = _table.erase(it);
      } else {
        ++it;
      }
    }

    #ifdef DEBUG
    log("neibor " + neibor_p->dest.to_string() + " removed");
    #endif

    Rip::remove_neibor(neibor_p);
  }

  void rip::Rip_distributed::receive_message(host_t source, std::string message) {
    std::cout << "Receive message from " << source.ip << ":" << source.port << " \"" << message << "\"" << std::endl;
  }

  void rip::Rip_distributed::receive_table(host_t source, table_t table) {

    #ifdef DEBUG
    log("receive table from " + source.to_string());
    #endif

    auto neibor_p = find_neibor(source);
    if (neibor_p != _neibors.end()) {
      update_neibor_timer(neibor_p);
    } else {
      neibor_p = add_neibor(source);
    }
    // using DV algorithm
    for (auto it = table.begin(); it != table.end(); ++it) {
      bool find = false;
      for (auto jt = _table.begin(); jt != _table.end() && !find; ++jt) {
        if (it->dest == jt->dest) {
          find = true;
          if (it->cost + 1 < jt->cost) {
            jt->cost = it->cost + 1;
            jt->next = source;
          }
        }
      }
      if (!find) {
        _table.push_back(table_item(it->dest, source, it->cost + 1));
      }
    }

  }