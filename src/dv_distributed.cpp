#include "dv_distributed.hpp"
#include <iostream>

#define DEBUG

#ifdef DEBUG
static void log(std::string message) {
  std::cout << "[dv dist]\t" << message << std::endl;
}
#endif

rip::Dv_distributed::Dv_distributed(std::string ip, Udp::port_t port) : Dv(ip, port) {}
rip::Dv_distributed::~Dv_distributed() {}

void rip::Dv_distributed::sync() {
  for (auto it = _neibors.begin(); it != _neibors.end(); ++it) {

    // #ifdef DEBUG
    // log("sync with " + it->dest.to_string());
    // #endif

    send_table(it->host);
  }
}

rip::Dv_distributed::neibor_ptr rip::Dv_distributed::add_neibor(host_t host) {
  auto neibor_p = Dv::add_neibor(host);
  update_neibor_timer(neibor_p);
  _table[host] = table_item(host, 1);

  #ifdef BUDEG
  log("neibor + " + neibor_p->dest.to_string() + " added");
  #endif

  return neibor_p;
}

void rip::Dv_distributed::remove_neibor(neibor_ptr neibor_p) {
  for (auto it = _table.begin(); it != _table.end();) {
    if (it->first == neibor_p->host || it->second.next == neibor_p->host) {
      it = _table.erase(it);
    } else {
      ++it;
    }
  }

  #ifdef DEBUG
  log("neibor " + neibor_p->host.to_string() + " removed");
  #endif

  Dv::remove_neibor(neibor_p);
}

void rip::Dv_distributed::receive_message(host_t source, std::string message) {
  std::cout << "Receive message from " << Udp::stringify_ip(source.ip) << ":" << source.port << " \"" << message << "\"" << std::endl;
}

void rip::Dv_distributed::route_message(host_t dest, std::string message) {
  Dv::route_message(dest, message);
  #ifdef DEBUG
  log("routing for " + dest.to_string() + " [" + message + "]");
  #endif
}

void rip::Dv_distributed::receive_table(host_t source, table_t table) {

  // #ifdef DEBUG
  // log("receive table from " + source.to_string());
  // #endif

  auto neibor_p = find_neibor(source);
  if (neibor_p != _neibors.end()) {
    update_neibor_timer(neibor_p);
  } else {
    neibor_p = add_neibor(source);
  }
  // using DV algorithm
  update_table(source, table);

  // #ifdef DEBUG
  // log("current table:");
  // for (auto it = _table.begin(); it != _table.end(); ++it) {
  //   log("|" + it->dest.to_string() + "\t|" + it->next.to_string() + "\t|" + std::to_string(it->cost) + "\t|");
  // }
  // #endif
}
