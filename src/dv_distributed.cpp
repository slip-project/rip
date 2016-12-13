#include "dv_distributed.hpp"
#include <iostream>

#define DEBUG

#ifdef DEBUG
static void log(std::string message) {
  std::cout << "[dv dist]\t" << message << std::endl;
}
#endif

rip::Dv_distributed::Dv_distributed(host_t localhost) : Dv(localhost) {}
rip::Dv_distributed::~Dv_distributed() {}

void rip::Dv_distributed::sync() {
  for (auto it = _neighbors.begin(); it != _neighbors.end(); ++it) {

    // #ifdef DEBUG
    // log("sync with " + it->dest.to_string());
    // #endif

    send_table(it->host);
  }
}

rip::Dv_distributed::neighbor_ptr rip::Dv_distributed::add_neighbor(host_t host) {
  auto neighbor_p = Dv::add_neighbor(host);
  update_neighbor_timer(neighbor_p);
  _table[host] = table_item(host, 1);

  #ifdef DEBUG
  log("neighbor + " + neighbor_p->host.to_string() + " added");
  #endif

  return neighbor_p;
}

void rip::Dv_distributed::remove_neighbor(neighbor_ptr neighbor_p) {
  for (auto it = _table.begin(); it != _table.end();) {
    if (it->first == neighbor_p->host || it->second.next == neighbor_p->host) {
      it = _table.erase(it);
    } else {
      ++it;
    }
  }

  #ifdef DEBUG
  log("neighbor " + neighbor_p->host.to_string() + " removed");
  #endif

  Dv::remove_neighbor(neighbor_p);
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

  auto neighbor_p = find_neighbor(source);
  if (neighbor_p != _neighbors.end()) {
    update_neighbor_timer(neighbor_p);
  } else {
    neighbor_p = add_neighbor(source);
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
