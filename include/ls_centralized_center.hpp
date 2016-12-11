#ifndef __LS__CENTRALIZED__CENTER__HPP__
#define __LS__CENTRALIZED__CENTER__HPP__ 1

#include "ls.hpp"

namespace rip {

class Ls_centralized_center: public Ls {

public:

  Ls_centralized_center(host_t localhost);
  virtual ~Ls_centralized_center();

  void sync() override;

  neibor_ptr add_neibor(host_t host) override;

  void remove_neibor(neibor_ptr neibor_p) override;

  void receive_message(host_t source, std::string message) override;

  void route_message(host_t dest, std::string message) override;

  void receive_table(host_t source, table_t table) override;

};

};

#endif
