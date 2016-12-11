#include "ls.hpp"
#include <regex>
#include <queue>
#include <sstream>
#include <functional>

// #define DEBUG

#ifdef DEBUG
#include <iostream>
static void log(std::string message) {
  std::cout << "[ls]\t" << message << std::endl;
}
#endif

rip::Ls::Ls(host_t localhost)
  : Rip(localhost) {}

rip::Ls::~Ls() {}

void rip::Ls::send_table(host_t host) {
  auto table_str = stringify_table(_table);

  int header_len = sizeof(rip_header);

  rip_header rip_h;

  rip_h.type = TABLE;
  rip_h.dest = host;

  std::string data = std::string((char *)&rip_h, header_len) + table_str;

  _udp.send(host.ip, host.port, data);
}

void rip::Ls::update_table(host_t source, table_t table) {
  // LS angorithm
  auto router = std::map<host_t, router_item>();
  std::priority_queue<router_item, std::vector<router_item>, std::greater<router_item>> q;
  _table = table;
  _table[_localhost] = {};
  // 首先将邻居加入路径表
  for (auto it = _neibors.begin(); it != _neibors.end(); ++it) {
    router[it->host] = std::make_pair(1, _localhost);
    q.push(std::make_pair(1, it->host));
    _table[_localhost].push_back(it->host);
  }
  // 然后运行 dijsktra 算法
  while (!q.empty()) {
    auto cur = q.top();
    q.pop();

    #ifdef DEBUG
    log("cur: (" + std::to_string(cur.first) + "," + cur.second.to_string() + ")");
    #endif

    for (auto it = table[cur.second].begin(); it != table[cur.second].end(); ++it) {

      #ifdef DEBUG
      log("iter: " + it->to_string());
      #endif

      if (*it != _localhost) {
        auto jt = router.find(*it);
        if (jt != router.end()) {
          if (jt->second.first > cur.first + 1) {
            jt->second = std::make_pair(cur.first + 1, cur.second);
            q.push(std::make_pair(cur.first + 1, *it));
          }
        } else {
          router[*it] = std::make_pair(cur.first + 1, cur.second);
          q.push(std::make_pair(cur.first + 1, *it));
        }
      }
    }
  }
  #ifdef DEBUG
  for (auto it = router.begin(); it != router.end(); ++it) {
    log(it->first.to_string() + " = (" + std::to_string(it->second.first) + " , " + it->second.second.to_string() + ")");
  }
  #endif
  // 路径计算完毕，开始计算每个节点的转发端口
  _router = router_t();
  while (router.size()) {
    for (auto it = router.begin(); it != router.end();) {
      if (it->second.second == _localhost) {
        _router[it->first] = it->first;
        it = router.erase(it);
      } else {
        auto jt = _router.find(it->second.second);
        if (jt != _router.end()) {
          _router[it->first] = jt->second;
          it = router.erase(it);
        } else {
          ++it;
        }
      }
    }
  }
}

void rip::Ls::route_message(host_t dest, std::string message) {
  auto it = _router.find(dest);
  if (it != _router.end()) {
    send_message(dest, it->second, message);
  }
}

std::string rip::Ls::stringify_table(table_t table) {
  std::stringstream stream;
  for (auto it = table.begin(); it != table.end(); ++it) {
    stream << "{" << it->first.to_string() << "=[";
    for (auto jt = it->second.begin(); jt != it->second.end(); ++jt) {
      stream << jt->to_string() << " ";
    }
    stream << "]}";
  }
  return stream.str();
}

rip::Ls::table_t rip::Ls::parse_table(std::string table_str) {

  #ifdef DEBUG
  log("parsing table");
  #endif

  const std::string host_pattern = "([0-9\\.]+):(\\d+)\\s*";
  const std::string map_pattern = "\\{" + host_pattern + "=\\[([^\\]]*)\\]\\}";

  const std::regex host_regex(host_pattern), map_regex(map_pattern);

  std::smatch sm;
  table_t table;

  while (table_str.size()) {
    if (!std::regex_search(table_str, sm, map_regex, std::regex_constants::match_continuous)) {
      throw std::runtime_error("parse error");
    }

    // #ifdef DEBUG
    // log("0:" + std::string(sm[0]));
    // log("1:" + std::string(sm[1]));
    // log("2:" + std::string(sm[2]));
    // log("3:" + std::string(sm[3]));
    // #endif

    host_t key(sm[1], std::stoi(sm[2]));
    std::string list_str = sm[3];
    table_str = sm.suffix().str();

    auto it = table.insert(table.end(), std::make_pair(key, std::list<host_t>()));

    while (list_str.size()) {
      if (!std::regex_search(list_str, sm, host_regex, std::regex_constants::match_continuous)) {
        throw std::runtime_error("parse error");
      }

      // #ifdef DEBUG
      // log("0:" + std::string(sm[0]));
      // log("1:" + std::string(sm[1]));
      // log("2:" + std::string(sm[2]));
      // #endif

      host_t host(sm[1], std::stoi(sm[2]));
      list_str = sm.suffix().str();

      it->second.push_back(host);
    }
  }

  return table;
}
