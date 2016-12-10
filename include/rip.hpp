#ifndef __RIP__HPP__
#define __RIP__HPP__ 1

#include "udp.hpp"
#include "timeout.hpp"

namespace rip {

class Rip {
public:

  static const int SYNC_INTERVAL = 1000;
  static const int WAIT_TIMEOUT  = 5000;

  static const short MESSAGE = 1;
  static const short TABLE   = 2;

  struct host_t {
    std::string ip;
    Udp::port_t port;
  };

  struct rip_header {
    short type;
    host_t dest;
  };

  struct table_item {
    host_t dest;
    host_t next;
    int cost;
  };

  struct neibor_t {
    host_t dest;
    Timeout::timer_pcb_ptr timer;
  };

  typedef std::list<table_item> table_t;

  typedef std::list<neibor_t> neibor_list;

  typedef neibor_list::iterator neibor_ptr;

  Rip(Udp::port_t port): _udp(port) {
    _timer = _timeout.add_timer(SYNC_INTERVAL, [=]()->void{

    })
  }
  ~Rip();

  neibor_ptr add_neibor(host_t host) {
    neibor_t neibor(host, nullptr);
    _neibors.push_front(neibor);
    neibor_ptr neibor_p = _neibors.begin();
    // auto timer_ptr = _timeout.add_timer(WAIT_TIMEOUT, [=]()->void{
    //   remove_neibor(neibor_p);
    // });
    // neibor_p->timer = timer_ptr;
    return neibor_p;
  }

  void remove_neibor(neibor_ptr neibor_p) {
    neibor_p->timer->enable = false;
    _neibors.erase(neibor_p);
  }

  void update_neibor_timer(neibor_ptr neibor_p) {
    if (neiebor_p->timer) neibor_p->timer->enable = false;
    neibor_p->timer = _timeout.add_timer(WAIT_TIMEOUT, [=]()->void{
      remove_neibor(neibor_p);
    });
  }

  void send_table(host_t host) {
    auto table_str = stringify_table(_table);
    int tot_len = sizeof(rip_header) + table_str.size();
    char *tmp = new char[tot_len];

    memcpy(tmp, tmp + sizeof(rip_header), table_str.size());
    rip_header *rip_h = (rip_header*) tmp;

    rip_h->type = TABLE;
    rip_h->dest = host;

    std::string data(tmp, tot_len);
    delete tmp;

    _udp.send(host.ip, host.port, data);
  }

  void receive_table(host_t host, table_t table) {
    // abstruct
  }

  void send_message(host_t host, std::string message) {
    int tot_len = sizeof(rip_header) + message.size();
    char *tmp = new char[tot_len];

    memcpy(tmp, tmp + sizeof(rip_header), message.size());
    rip_header *rip_h = (rip_header*) tmp;

    rip_h->type = MESSAGE;
    rip_h->dest = host;

    std::string data(tmp, tot_len);
    delete tmp;

    _udp.send(host.ip, host.port, data);
  }

  void receive_message(host_t host, std::string message) {
    // abstruct
  }

  std::string stringify_table(table_t table) {
    int tot_len = sizeof(table_item) * table.size(), index = 0;
    char *tmp = new char[tot_len];
    for (auto it = _neibors.begin(); it != _neibors.end(); ++it, ++index) {
      memcpy(tmp + index * sizeof(table_item), &(*it), sizeof(table_item));
    }
    std::string table_str(tmp, tot_len);
    delete tmp;
    return table_str;
  }

  table_t parse_table(std::string table_str) {
    char *tmp = table_str.data();
    table_item item;
    table_t table;
    for (int i = 0; i < table_str.size(); i += sizeof(table_item)) {
      memcpy(&item, tmp + i, sizeof(table_item));
      table.push_back(item);
    }
    return table;
  }

protected:
  table_t _table;
  neibor_list _neibors;
  Udp _udp;
  Timeout _timeout;

};

};

#endif // __RIP__HPP__
