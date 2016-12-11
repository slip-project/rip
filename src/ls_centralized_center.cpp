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
  for (auto it = _neibors.begin(); it != _neibors.end(); ++it) {

    // #ifdef DEBUG
    // log("sync with " + it->dest.to_string());
    // #endif

    send_table(it->host);
  }
}

rip::Ls_centralized_center::neibor_ptr rip::Ls_centralized_center::add_neibor(host_t host) {
  auto neibor_p = Ls::add_neibor(host);
  update_neibor_timer(neibor_p);

  #ifdef DEBUG
  log("neibor + " + neibor_p->host.to_string() + " added");
  #endif

  return neibor_p;
}

void rip::Ls_centralized_center::remove_neibor(neibor_ptr neibor_p) {
  _table.erase(neibor_p->host);

  #ifdef DEBUG
  log("neibor " + neibor_p->host.to_string() + " removed");
  #endif

  Ls::remove_neibor(neibor_p);
}

void rip::Ls_centralized_center::receive_message(host_t source, std::string message) {}

void rip::Ls_centralized_center::route_message(host_t dest, std::string message) {}

void rip::Ls_centralized_center::receive_table(host_t source, table_t table) {
  _table[source] = table[source];
  auto neibor_p = find_neibor(source);
  if (neibor_p != _neibors.end()) {
    update_neibor_timer(neibor_p);
  } else {
    add_neibor(source);
  }
}
