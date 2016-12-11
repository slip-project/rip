#include "ls_distributed.hpp"
#include <iostream>

// #define DEBUG

#ifdef DEBUG
static void log(std::string message) {
  std::cout << "[ls dist]\t" << message << std::endl;
}
#endif

rip::Ls_distributed::Ls_distributed(host_t localhost) : Ls(localhost) {}
rip::Ls_distributed::~Ls_distributed() {}

void rip::Ls_distributed::sync() {
  for (auto it = _neibors.begin(); it != _neibors.end(); ++it) {

    // #ifdef DEBUG
    // log("sync with " + it->host.to_string());
    // #endif

    send_table(it->host);
  }
}

rip::Ls_distributed::neibor_ptr rip::Ls_distributed::add_neibor(host_t host) {
  auto neibor_p = Ls::add_neibor(host);
  update_neibor_timer(neibor_p);

  #ifdef DEBUG
  log("neibor + " + neibor_p->host.to_string() + " added");
  #endif

  return neibor_p;
}

void rip::Ls_distributed::receive_message(host_t source, std::string message) {
  std::cout << "Receive message from " << Udp::stringify_ip(source.ip) << ":" << source.port << " \"" << message << "\"" << std::endl;
}

void rip::Ls_distributed::route_message(host_t dest, std::string message) {
  Ls::route_message(dest, message);
  #ifdef DEBUG
  log("routing for " + dest.to_string() + " [" + message + "]");
  #endif
}

void rip::Ls_distributed::receive_table(host_t source, table_t table) {

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
  update_table(source, table);

  // #ifdef DEBUG
  // log("current table:");
  // for (auto it = _table.begin(); it != _table.end(); ++it) {
  //   log("|" + it->dest.to_string() + "\t|" + it->next.to_string() + "\t|" + std::to_string(it->cost) + "\t|");
  // }
  // #endif
}
