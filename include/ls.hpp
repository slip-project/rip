#ifndef __LS__HPP__
#define __LS__HPP__ 1

#include "rip.hpp"
#include <map>
#include <list>

namespace rip {

class Ls: public Rip<std::map<host_t, std::list<host_t>>> {
public:

  typedef std::pair<int, host_t> router_item;
  typedef std::map<host_t, host_t> router_t;

  Ls(host_t localhost);

  virtual ~Ls();

  void send_table(host_t host) override;

  void update_table(host_t host, table_t table) override;

  void route_message(host_t dest, std::string message) override;

  std::string stringify_table(table_t table) override;

  table_t parse_table(std::string table_str) override;

  router_t get_router() { return _router; }

protected:

  router_t _router;

};

};

#endif // __LS__HPP__
