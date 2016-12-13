#ifndef __LS__DISTRIBUTED__HPP__
#define __LS__DISTRIBUTED__HPP__ 1

#include "ls.hpp"
#include <string>

namespace rip {

class Ls_distributed: public Ls {

public:

  Ls_distributed(host_t localhost);
  virtual ~Ls_distributed();

  void sync() override;

  neighbor_ptr add_neighbor(host_t host) override;

  void receive_message(host_t source, std::string message) override;

  void route_message(host_t dest, std::string message) override;

  void receive_table(host_t source, table_t table) override;

};

};

#endif
