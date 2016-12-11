#include "dv.hpp"
#include <cstdlib>

// #define DEBUG

#ifdef DEBUG
#include <iostream>
static void log(std::string message) {
  std::cout << "[dv]\t" << message << std::endl;
}
#endif

rip::Dv::table_item::table_item(host_t nn, int cc)
  : next(nn), cost(cc) {}

rip::Dv::Dv(std::string ip, Udp::port_t port) : Rip(ip, port) {
  _table[_localhost] = table_item(_localhost, 0);
}

rip::Dv::~Dv() {}

void rip::Dv::send_table(host_t host) {
  auto table_str = stringify_table(_table);

  int header_len = sizeof(rip_header);

  rip_header rip_h;

  rip_h.type = TABLE;
  rip_h.dest = host;

  std::string data = std::string((char *)&rip_h, header_len) + table_str;

  _udp.send(host.ip, host.port, data);
}

void rip::Dv::update_table(host_t source, table_t table) {
  // DV algorithm
  for (auto it = table.begin(); it != table.end(); ++it) {
    if (it->second.next != _localhost) {
    // 当邻居的转发表中的 next 不为自身时才更新
      auto jt = _table.find(it->first);
      if (jt != _table.end()) {
        // 如果有距离更新，则更新转发表
        if (it->second.cost + 1 < jt->second.cost) {
          jt->second.cost = it->second.cost + 1;
          jt->second.next = source;
        }
      } else {
        // 如果有新的节点，则加入转发表
        _table[it->first] = table_item(source, it->second.cost + 1);
      }
    }
  }

  // 如果有某个转发项的 next 为 source，但是新的转发表中缺失，则判断失效
  for (auto it = _table.begin(); it != _table.end();) {
    if (it->second.next == source) {
      auto jt = table.find(it->first);
      if (jt == table.end()) {
        it = _table.erase(it);
        continue;
      }
    }
    ++it;
  }
}

void rip::Dv::route_message(host_t dest, std::string message) {
  auto it = _table.find(dest);
  if (it != _table.end()) {
    send_message(dest, it->second.next, message);
  }
}

std::string rip::Dv::stringify_table(table_t table) {
  int item_len = sizeof(table_t::value_type);
  int tot_len = item_len * table.size(), index = 0;
  char *tmp = new char[tot_len];
  for (auto it = table.begin(); it != table.end(); ++it, ++index) {
    memcpy(tmp + index * item_len, &(*it), item_len);
  }
  std::string table_str(tmp, tot_len);
  delete[] tmp;
  return table_str;
}

rip::Dv::table_t rip::Dv::parse_table(std::string table_str) {

  #ifdef DEBUG
  log("parsing table");
  #endif

  int item_len = sizeof(table_t::value_type);
  const char *tmp = table_str.data();
  table_t::value_type item;
  table_t table;
  for (int i = 0; i < table_str.size(); i += item_len) {

    #ifdef DEBUG
    log(std::to_string(i) + "/" + std::to_string(table_str.size()));
    #endif

    memcpy(&item, tmp + i, item_len);
    table[item.first] = item.second;
  }
  return table;
}
