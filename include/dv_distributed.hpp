#ifndef __DV__DISTRIBUTED__HPP__
#define __DV__DISTRIBUTED__HPP__ 1

#include "dv.hpp"
#include <string>

namespace rip {

class Dv_distributed: public Dv {

public:

  Dv_distributed(host_t localhost);
  virtual ~Dv_distributed();

  void sync() override;

  neighbor_ptr add_neighbor(host_t host) override;

  void remove_neighbor(neighbor_ptr neighbor_p) override;

  void receive_message(host_t source, std::string message) override;

  void route_message(host_t dest, std::string message) override;

  void receive_table(host_t source, table_t table) override;

};

};

#endif
