#ifndef __RIP__HPP__
#define __RIP__HPP__ 1

#include "udp.hpp"
#include "timeout.hpp"
#include <string>

namespace rip {

struct host_t {
  Udp::ip_t ip;
  Udp::port_t port;

  host_t(std::string ii, Udp::port_t pp)
    : ip(Udp::parse_ip(ii)), port(pp) {}
  host_t(Udp::ip_t ii, Udp::port_t pp)
    : ip(ii), port(pp) {}
  host_t() = default;

  bool operator==(const host_t & other) const {
    return ip == other.ip && port == other.port;
  }

  bool operator!=(const host_t & other) const {
    return !operator==(other);
  }

  bool operator<(const host_t & other) const {
    return ip < other.ip || (ip == other.ip && port < other.port);
  }

  std::string to_string() const {
    return Udp::stringify_ip(ip) + ":" + std::to_string(port);
  }
};

template<class T>
class Rip {
public:

  static const int SYNC_INTERVAL = 1000;
  static const int WAIT_TIMEOUT  = 5000;

  static const short MESSAGE = 1;
  static const short TABLE   = 2;
  static const short HEART   = 3;

  struct rip_header {
    short type;
    host_t dest;
  };

  struct neibor_t {
    host_t host;
    Timeout::timer_pcb_ptr timer;

    neibor_t(host_t hh)
      : host(hh) {}
    neibor_t() = default;
  };

  typedef T table_t;

  typedef std::list<neibor_t> neibor_list;

  typedef typename neibor_list::iterator neibor_ptr;

  Rip(std::string ip, Udp::port_t port): _localhost(ip, port), _udp(port) {
    schedule_sync();
    _udp.add_listener([=](Udp::ip_t source_ip, Udp::port_t source_port, std::string data)->void{
      solve_comming_message(source_ip, source_port, data);
    });
  }

  virtual ~Rip() {
    if (_timer) _timer->enable = false;
    for (auto it = _neibors.begin(); it != _neibors.end(); ++it) {
      if (it->timer) it->timer->enable = false;
    }
  }

  void schedule_sync()  {

    #ifdef DEBUG
    log("schedule sync");
    #endif

    if (_timer) _timer->enable = false;
    _timer = _timeout.add_timer(SYNC_INTERVAL, [=]()->void{
      sync();
      schedule_sync();
    });
  }

  virtual void sync() = 0;

  void solve_comming_message(Udp::ip_t source_ip, Udp::port_t source_port, std::string data) {

    int header_len = sizeof(rip_header);

    host_t source_host(source_ip, source_port);
    rip_header rip_h(*((const rip_header*) data.data()));
    std::string message = data.substr(header_len);

    #ifdef DEBUG
    log("solve comming message from " + source_host.to_string()
        + " type:" + std::to_string(rip_h.type) + " dest: " + rip_h.dest.to_string());
    #endif

    switch (rip_h.type) {
      case MESSAGE:
        if (rip_h.dest == _localhost) {
          receive_message(source_host, message);
        } else {
          route_message(rip_h.dest, message);
        }
        break;
      case TABLE:
        receive_table(source_host, parse_table(message));
        break;
      case HEART:
        receive_heart_beat(source_host);
        break;
    }
  }

  virtual neibor_ptr add_neibor(host_t host) {
    neibor_t neibor(host);
    _neibors.push_front(neibor);
    neibor_ptr neibor_p = _neibors.begin();
    return neibor_p;
  }

  neibor_ptr find_neibor(host_t host) {
    for (neibor_ptr it = _neibors.begin(); it != _neibors.end(); ++it) {
      if (it->host == host) {
        return it;
      }
    }
    return _neibors.end();
  }

  virtual void remove_neibor(neibor_ptr neibor_p) {
    neibor_p->timer->enable = false;
    _neibors.erase(neibor_p);
  }

  void update_timer(host_t host)  {
    auto neibor_p = find_neibor(host);
    if (neibor_p != _neibors.end()) update_neibor_timer(neibor_p);
  }

  void update_neibor_timer(neibor_ptr neibor_p)  {
    if (neibor_p->timer) neibor_p->timer->enable = false;
    neibor_p->timer = _timeout.add_timer(WAIT_TIMEOUT, [=]()->void{

      #ifdef DEBUG
      log("timeout triggerd for " + neibor_p->dest.to_string());
      #endif

      remove_neibor(neibor_p);
    });
  }

  virtual void send_table(host_t host) = 0;

  virtual void receive_table(host_t host, table_t table) = 0;
    void send_message(host_t dest, host_t router, std::string message) {
    int header_len = sizeof(rip_header);

    rip_header rip_h;

    rip_h.type = MESSAGE;
    rip_h.dest = dest;

    std::string data = std::string((char *)&rip_h, header_len) + message;

    _udp.send(router.ip, router.port, data);
  }

  virtual void route_message(host_t dest, std::string message) = 0;

  virtual void receive_message(host_t host, std::string message) = 0;

  void send_heart_beat(host_t dest) {
    int tot_len = sizeof(rip_header);

    rip_header rip_h;

    rip_h->type = HEART;
    rip_h->dest = dest;

    std::string data((char *)&rip_h, tot_len);

    _udp.send(dest.ip, dest.port, data);
  }

  void receive_heart_beat(host_t host)  {
    update_timer(host);
  }

  const table_t & get_table() const { return _table; }

  virtual std::string stringify_table(table_t table) = 0;

  virtual table_t parse_table(std::string table_str) = 0;

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
