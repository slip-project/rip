#include "rip.hpp"
#include <cstdlib>

// #define DEBUG

#ifdef DEBUG
#include <iostream>
static void log(std::string message) {
  std::cout << "[rip]\t" << message << std::endl;
}
#endif

rip::Rip::host_t::host_t(std::string ii, Udp::port_t pp) :
  ip(Udp::parse_ip(ii)), port(pp) {}

rip::Rip::host_t::host_t(Udp::ip_t ii, Udp::port_t pp) :
  ip(ii), port(pp) {}

bool rip::Rip::host_t::operator==(const host_t & other) const {
  return ip == other.ip && port == other.port;
}

bool rip::Rip::host_t::operator!=(const host_t & other) const {
  return !operator==(other);
}

std::string rip::Rip::host_t::to_string() const {
  return Udp::stringify_ip(ip) + ":" + std::to_string(port);
}

rip::Rip::neibor_t::neibor_t(host_t dd): dest(dd) {}

rip::Rip::table_item::table_item(host_t dd, host_t nn, int cc)
  : dest(dd), next(nn), cost(cc) {}

rip::Rip::Rip(std::string ip, Udp::port_t port): _localhost(ip, port), _udp(port) {
  _table.push_back(table_item(_localhost, _localhost, 0));
  schedule_sync();
  _udp.add_listener([=](Udp::ip_t source_ip, Udp::port_t source_port, std::string data)->void{
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

void rip::Rip::solve_comming_message(Udp::ip_t source_ip, Udp::port_t source_port, std::string data) {

  int header_len = sizeof(rip_header);

  host_t source_host(source_ip, source_port);
  rip_header rip_h(*((const rip_header*) data.data()));
  std::string message = data.substr(header_len);

  #ifdef DEBUG
  log("solve comming message from " + source_host.to_string()
      + " type:" + std::to_string(rip_h.type) + " dest: " + rip_h.dest.to_string());
  #endif

  switch (rip_h.type) {
    case MESSAGE:
      if (rip_h.dest == _localhost) {
        receive_message(source_host, message);
      } else {
        route_message(rip_h.dest, message);
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

bool rip::Rip::has_neibor(host_t host) const {
  for (auto it = _neibors.begin(); it != _neibors.end(); ++it) {
    if (it->dest == host) {
      return true;
    }
  }
  return false;
}

rip::Rip::neibor_ptr rip::Rip::find_neibor(host_t host) {
  for (neibor_ptr it = _neibors.begin(); it != _neibors.end(); ++it) {
    if (it->dest == host) {
      return it;
    }
  }
  return _neibors.end();
}

void rip::Rip::remove_neibor(neibor_ptr neibor_p) {
  neibor_p->timer->enable = false;
  _neibors.erase(neibor_p);
}

void rip::Rip::update_timer(host_t host) {
  auto neibor_p = find_neibor(host);
  if (neibor_p != _neibors.end()) update_neibor_timer(neibor_p);
}

void rip::Rip::update_neibor_timer(neibor_ptr neibor_p) {
  if (neibor_p->timer) neibor_p->timer->enable = false;
  neibor_p->timer = _timeout.add_timer(WAIT_TIMEOUT, [=]()->void{

    #ifdef DEBUG
    log("timeout triggerd for " + neibor_p->dest.to_string());
    #endif

    remove_neibor(neibor_p);
  });
}

void rip::Rip::send_table(host_t host) {
  auto table_str = stringify_table(_table);

  int header_len = sizeof(rip_header);

  rip_header rip_h;

  rip_h.type = TABLE;
  rip_h.dest = host;

  std::string data = std::string((char *)&rip_h, header_len) + table_str;

  _udp.send(host.ip, host.port, data);
}

void rip::Rip::send_message(host_t dest, host_t router, std::string message) {
  int header_len = sizeof(rip_header);

  rip_header rip_h;

  rip_h.type = MESSAGE;
  rip_h.dest = dest;

  std::string data = std::string((char *)&rip_h, header_len) + message;

  _udp.send(router.ip, router.port, data);
}

void rip::Rip::route_message(host_t dest, std::string message) {
  for (auto it = _table.begin(); it != _table.end(); ++it) {
    if (it->dest == dest) {
      send_message(dest, it->next, message);
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
  for (auto it = table.begin(); it != table.end(); ++it, ++index) {
    memcpy(tmp + index * sizeof(table_item), &(*it), sizeof(table_item));
  }
  std::string table_str(tmp, tot_len);
  delete[] tmp;
  return table_str;
}

rip::Rip::table_t rip::Rip::parse_table(std::string table_str) {

  #ifdef DEBUG
  log("parsing table");
  #endif

  const char *tmp = table_str.data();
  table_item item;
  table_t table;
  for (int i = 0; i < table_str.size(); i += sizeof(table_item)) {

    #ifdef DEBUG
    log(std::to_string(i) + "/" + std::to_string(table_str.size()));
    #endif

    memcpy(&item, tmp + i, sizeof(table_item));
    table.push_back(item);
  }
  return table;
}
