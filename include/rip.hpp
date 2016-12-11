#ifndef __RIP__HPP__
#define __RIP__HPP__ 1

#include "udp.hpp"
#include "timeout.hpp"
#include <string>

namespace rip {

class Rip {
public:

  static const int SYNC_INTERVAL = 1000;
  static const int WAIT_TIMEOUT  = 5000;

  static const short MESSAGE = 1;
  static const short TABLE   = 2;
  static const short HEART   = 3;

  struct host_t {
    Udp::ip_t ip;
    Udp::port_t port;

    host_t(std::string ip, Udp::port_t port);
    host_t(Udp::ip_t ip, Udp::port_t port);
    host_t() = default;
    bool operator==(const host_t & other) const;
    bool operator!=(const host_t & other) const;
    std::string to_string() const;
  };

  struct rip_header {
    short type;
    host_t dest;
  };

  struct table_item {
    host_t dest;
    host_t next;
    int cost;

    table_item(host_t dest, host_t next, int cost);
    table_item() = default;
  };

  struct neibor_t {
    host_t dest;
    Timeout::timer_pcb_ptr timer;

    neibor_t(host_t dest);
  };

  typedef std::list<table_item> table_t;

  typedef std::list<neibor_t> neibor_list;

  typedef neibor_list::iterator neibor_ptr;

  Rip(std::string ip, Udp::port_t port);

  virtual ~Rip();

  void schedule_sync();

  virtual void sync() = 0;

  void solve_comming_message(Udp::ip_t source_ip, Udp::port_t source_port, std::string data);

  virtual neibor_ptr add_neibor(host_t host);

  bool has_neibor(host_t host) const;

  neibor_ptr find_neibor(host_t host);

  virtual void remove_neibor(neibor_ptr neibor_p);

  void update_timer(host_t host);

  void update_neibor_timer(neibor_ptr neibor_p);

  void send_table(host_t host);

  virtual void receive_table(host_t host, table_t table) = 0;

  void send_message(host_t dest, host_t router, std::string message);

  virtual void route_message(host_t dest, std::string message);

  virtual void receive_message(host_t host, std::string message) = 0;

  void send_heart_beat(host_t host);

  void receive_heart_beat(host_t host);

  static std::string stringify_table(table_t table);

  static table_t parse_table(std::string table_str);

  const table_t & get_table() const { return _table; }

protected:
  host_t _localhost;
  table_t _table;
  neibor_list _neibors;
  Udp _udp;
  Timeout _timeout;
  Timeout::timer_pcb_ptr _timer;

};

};

#endif // __RIP__HPP__
