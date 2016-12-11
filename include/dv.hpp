#ifndef __DV__HPP__
#define __DV__HPP__ 1

#include "rip.hpp"
#include <map>

namespace rip {

static struct table_item {
  host_t next;
  int cost;

  table_item(host_t next, int cost);
  table_item() = default;
};

class Dv: public Rip<std::map<host_t, table_item> > {
public:

  typedef table_item table_item;

  Dv(std::string ip, Udp::port_t port);

  virtual ~Dv();

  void send_table(host_t host) override;

  virtual void update_table(host_t host, table_t table);

  void route_message(host_t dest, std::string message) override;

  std::string stringify_table(table_t table) override;

  table_t parse_table(std::string table_str) override;

};

};

#endif // __DV__HPP__
