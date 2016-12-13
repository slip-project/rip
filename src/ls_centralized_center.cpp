#include "ls_centralized_center.hpp"

#define DEBUG

#ifdef DEBUG
#include <iostream>
static void log(std::string message) {
  std::cout << "[ls cent]\t" << message << std::endl;
}
#endif

rip::Ls_centralized_center::Ls_centralized_center(host_t localhost) : Ls(localhost) {}
rip::Ls_centralized_center::~Ls_centralized_center() {}

void rip::Ls_centralized_center::sync() {
  for (auto it = _neighbors.begin(); it != _neighbors.end(); ++it) {

    // #ifdef DEBUG
    // log("sync with " + it->dest.to_string());
    // #endif

    send_table(it->host);
  }
}

rip::Ls_centralized_center::neighbor_ptr rip::Ls_centralized_center::add_neighbor(host_t host) {
  auto neighbor_p = Ls::add_neighbor(host);
  update_neighbor_timer(neighbor_p);

  #ifdef DEBUG
  log("neighbor + " + neighbor_p->host.to_string() + " added");
  #endif

  return neighbor_p;
}

void rip::Ls_centralized_center::remove_neighbor(neighbor_ptr neighbor_p) {
  _table.erase(neighbor_p->host);

  #ifdef DEBUG
  log("neighbor " + neighbor_p->host.to_string() + " removed");
  #endif

  Ls::remove_neighbor(neighbor_p);
}

void rip::Ls_centralized_center::receive_message(host_t source, std::string message) {}

void rip::Ls_centralized_center::route_message(host_t dest, std::string message) {}

void rip::Ls_centralized_center::receive_table(host_t source, table_t table) {
  _table[source] = table[source];
  auto neighbor_p = find_neighbor(source);
  if (neighbor_p != _neighbors.end()) {
    update_neighbor_timer(neighbor_p);
  } else {
    add_neighbor(source);
  }
}
