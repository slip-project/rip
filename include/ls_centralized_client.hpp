#ifndef __LS__CENTRALIZED__CLIENT__HPP__
#define __LS__CENTRALIZED__CLIENT__HPP__ 1

#include "ls.hpp"

namespace rip {

class Ls_centralized_client: public Ls {

public:

  Ls_centralized_client(host_t localhost, host_t center_host);
  virtual ~Ls_centralized_client();

  void sync() override;

  neighbor_ptr add_neighbor(host_t host) override;

  void remove_neighbor(neighbor_ptr neighbor_p) override;

  void receive_message(host_t source, std::string message) override;

  void route_message(host_t dest, std::string message) override;

  void send_table(host_t dest) override;

  void receive_table(host_t source, table_t table) override;

protected:

  host_t _center;

};

};

#endif
