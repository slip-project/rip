#include "rip.hpp"
#include <cstdlib>

#define DEBUG

#ifdef DEBUG
#include <iostream>
void log(std::string message) {
  std::cout << "[rip]\t" << message << std::endl;
}
#endif

rip::Rip::host_t::host_t(std::string ii, Udp::port_t pp) :
  ip(ii), port(pp) {}

bool rip::Rip::host_t::operator==(const host_t & other) const {
  return ip == other.ip && port == other.port;
}

rip::Rip::neibor_t::neibor_t(host_t dd): dest(dd) {}

rip::Rip::Rip(std::string ip, Udp::port_t port): _localhost(ip, port), _udp(port) {
  schedule_sync();
  _udp.add_listener([=](std::string source_ip, Udp::port_t source_port, std::string data)->void{
    solve_comming_message(source_ip, source_port, data);
  });
}

rip::Rip::~Rip() {
  if (_timer) _timer->enable = false;
  for (auto it = _neibors.begin(); it != _neibors.end(); ++it) {
    if (it->timer) it->timer->enable = false;
  }
}

void rip::Rip::schedule_sync() {

  #ifdef DEBUG
  log("schedule sync");
  #endif

  if (_timer) _timer->enable = false;
  _timer = _timeout.add_timer(SYNC_INTERVAL, [=]()->void{
    sync();
    schedule_sync();
  });
}

void rip::Rip::solve_comming_message(std::string source_ip, Udp::port_t source_port, std::string data) {

  host_t source_host(source_ip, source_port);
  const rip_header *rip_h = (const rip_header*) data.data();
  std::string message = data.substr(sizeof(rip_header));

  #ifdef DEBUG
  log("solve comming message from " + source_ip + ":" + std::to_string(source_port)
      + " type:" + std::to_string(rip_h->type) + " dest: " + rip_h->dest.ip + ":" + std::to_string(rip_h->dest.port)
      + " message:" + message);
  #endif

  switch (rip_h->type) {
    case MESSAGE:
      if (rip_h->dest == _localhost) {
        receive_message(source_host, message);
      } else {
        route_message(rip_h->dest, message);
      }
      break;
    case TABLE:
      receive_table(source_host, parse_table(message));
      break;
    case HEART:
      receive_heart_beat(source_host);
      break;
  }
}

rip::Rip::neibor_ptr rip::Rip::add_neibor(host_t host) {
  neibor_t neibor(host);
  _neibors.push_front(neibor);
  neibor_ptr neibor_p = _neibors.begin();
  return neibor_p;
}

void rip::Rip::remove_neibor(neibor_ptr neibor_p) {
  neibor_p->timer->enable = false;
  _neibors.erase(neibor_p);
}

void rip::Rip::update_timer(host_t host) {
  for (auto it = _neibors.begin(); it != _neibors.end(); ++it) {
    if (it->dest == host) {
      update_neibor_timer(it);
      return;
    }
  }
}

void rip::Rip::update_neibor_timer(neibor_ptr neibor_p) {
  if (neibor_p->timer) neibor_p->timer->enable = false;
  neibor_p->timer = _timeout.add_timer(WAIT_TIMEOUT, [=]()->void{

    #ifdef DEBUG
    log("timeout triggerd for " + neibor_p->dest.ip + std::to_string(neibor_p->dest.port));
    #endif

    remove_neibor(neibor_p);
  });
}

void rip::Rip::send_table(host_t host) {
  auto table_str = stringify_table(_table);
  int tot_len = sizeof(rip_header) + table_str.size();
  char *tmp = new char[tot_len];

  memcpy(tmp, tmp + sizeof(rip_header), table_str.size());
  rip_header *rip_h = (rip_header*) tmp;

  rip_h->type = TABLE;
  rip_h->dest = host;

  std::string data(tmp, tot_len);
  delete[] tmp;

  _udp.send(host.ip, host.port, data);
}

void rip::Rip::send_message(host_t dest, std::string message) {
  int tot_len = sizeof(rip_header) + message.size();
  char *tmp = new char[tot_len];

  memcpy(tmp, tmp + sizeof(rip_header), message.size());
  rip_header *rip_h = (rip_header*) tmp;

  rip_h->type = MESSAGE;
  rip_h->dest = dest;

  std::string data(tmp, tot_len);
  delete[] tmp;

  _udp.send(dest.ip, dest.port, data);
}

void rip::Rip::route_message(host_t dest, std::string message) {
  for (auto it = _table.begin(); it != _table.end(); ++it) {
    if (it->dest == dest) {
      send_message(it->next, message);
      break;
    }
  }
}

void rip::Rip::send_heart_beat(host_t host) {
  int tot_len = sizeof(rip_header);
  char *tmp = new char[tot_len];

  rip_header *rip_h = (rip_header*) tmp;

  rip_h->type = HEART;
  rip_h->dest = host;

  std::string data(tmp, tot_len);
  delete[] tmp;

  _udp.send(host.ip, host.port, data);
}

void rip::Rip::receive_heart_beat(host_t host) {
  update_timer(host);
}

std::string rip::Rip::stringify_table(table_t table) {
  int tot_len = sizeof(table_item) * table.size(), index = 0;
  char *tmp = new char[tot_len];
  for (auto it = _neibors.begin(); it != _neibors.end(); ++it, ++index) {
    memcpy(tmp + index * sizeof(table_item), &(*it), sizeof(table_item));
  }
  std::string table_str(tmp, tot_len);
  delete[] tmp;
  return table_str;
}

rip::Rip::table_t rip::Rip::parse_table(std::string table_str) {
  const char *tmp = table_str.data();
  table_item item;
  table_t table;
  for (int i = 0; i < table_str.size(); i += sizeof(table_item)) {
    memcpy(&item, tmp + i, sizeof(table_item));
    table.push_back(item);
  }
  return table;
}
