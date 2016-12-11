#ifndef __RIP__DISTRIBUTED__HPP__
#define __RIP__DISTRIBUTED__HPP__ 1

#include "rip.hpp"
#include <string>

namespace rip {

class Rip_distributed: public Rip {

public:

  Rip_distributed(std::string ip, Udp::port_t port);
  virtual ~Rip_distributed();

  void sync() override;

  neibor_ptr add_neibor(host_t host) override;

  void remove_neibor(neibor_ptr neibor_p) override;

  void receive_message(host_t source, std::string message) override;

  void route_message(host_t dest, std::string message) override;

  void receive_table(host_t source, table_t table) override;

};

};

#endif
