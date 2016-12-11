#include "ls_centralized_client.hpp"

#define DEBUG

#ifdef DEBUG
#include <iostream>
static void log(std::string message) {
  std::cout << "[ls cent]\t" << message << std::endl;
}
#endif

rip::Ls_centralized_client::Ls_centralized_client(host_t localhost, host_t center_host)
  : Ls(localhost), _center(center_host) {}
rip::Ls_centralized_client::~Ls_centralized_client() {}

void rip::Ls_centralized_client::sync() {
  send_table(_center);
  for (auto it = _neibors.begin(); it != _neibors.end(); ++it) {

    // #ifdef DEBUG
    // log("sync with " + it->dest.to_string());
    // #endif

    send_heart_beat(it->host);
  }
}

rip::Ls_centralized_client::neibor_ptr rip::Ls_centralized_client::add_neibor(host_t host) {
  auto neibor_p = Ls::add_neibor(host);
  update_neibor_timer(neibor_p);

  #ifdef DEBUG
  log("neibor + " + neibor_p->host.to_string() + " added");
  #endif

  return neibor_p;
}

void rip::Ls_centralized_client::remove_neibor(neibor_ptr neibor_p) {
  _table.erase(neibor_p->host);

  #ifdef DEBUG
  log("neibor " + neibor_p->host.to_string() + " removed");
  #endif

  Ls::remove_neibor(neibor_p);
}

void rip::Ls_centralized_client::receive_message(host_t source, std::string message) {
  std::cout << "Receive message from " << Udp::stringify_ip(source.ip) << ":" << source.port << " \"" << message << "\"" << std::endl;
}

void rip::Ls_centralized_client::route_message(host_t dest, std::string message) {
  Ls::route_message(dest, message);
  #ifdef DEBUG
  log("routing for " + dest.to_string() + " [" + message + "]");
  #endif
}

void rip::Ls_centralized_client::send_table(host_t dest) {
  table_t table;
  for (auto it = _neibors.begin(); it != _neibors.end(); ++it) {
    table[_localhost].push_back(it->host);
  }
  auto table_str = stringify_table(table);

  int header_len = sizeof(rip_header);

  rip_header rip_h;

  rip_h.type = TABLE;
  rip_h.dest = dest;

  std::string data = std::string((char *)&rip_h, header_len) + table_str;

  _udp.send(dest.ip, dest.port, data);
}

void rip::Ls_centralized_client::receive_table(host_t source, table_t table) {
  // #ifdef DEBUG
  // log("receive table from " + source.to_string());
  // #endif

  if (source == _center) {
    // using DV algorithm
    update_table(source, table);
  }
}
