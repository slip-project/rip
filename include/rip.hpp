#ifndef __RIP__HPP__
#define __RIP__HPP__ 1

#include "udp.hpp"
#include "timeout.hpp"
#include <string>

namespace rip {
/**
 * 主机类型
 */
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
  // 消息类型
  static const short MESSAGE = 1;  // 消息正文
  static const short TABLE   = 2;  // 路由表泛洪
  static const short HEART   = 3;  // 心跳
  // 路由消息头
  struct rip_header {
    short type;
    host_t dest;
  };
  // 邻居主机类型
  struct neighbor_t {
    host_t host;
    Timeout::timer_pcb_ptr timer;

    neighbor_t(host_t hh)
      : host(hh) {}
    neighbor_t() = default;
  };
  // 路由表类型, 模板由子类确定
  typedef T table_t;
  // 邻居列表
  typedef std::list<neighbor_t> neighbor_list;

  typedef typename neighbor_list::iterator neighbor_ptr;

  Rip(host_t localhost): _localhost(localhost), _udp(localhost.port) {
    schedule_sync();
    _udp.add_listener([=](Udp::ip_t source_ip, Udp::port_t source_port, std::string data)->void{
      solve_comming_message(source_ip, source_port, data);
    });
  }

  virtual ~Rip() {
    if (_timer) _timer->enable = false;
    for (auto it = _neighbors.begin(); it != _neighbors.end(); ++it) {
      if (it->timer) it->timer->enable = false;
    }
  }
  // 开始计时器
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
  // 处理接收到的消息
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
  // 添加邻居主机
  virtual neighbor_ptr add_neighbor(host_t host) {
    neighbor_t neighbor(host);
    _neighbors.push_front(neighbor);
    neighbor_ptr neighbor_p = _neighbors.begin();
    return neighbor_p;
  }
  // 在列表中查找邻居主机
  neighbor_ptr find_neighbor(host_t host) {
    for (neighbor_ptr it = _neighbors.begin(); it != _neighbors.end(); ++it) {
      if (it->host == host) {
        return it;
      }
    }
    return _neighbors.end();
  }
  // 移除邻居
  virtual void remove_neighbor(neighbor_ptr neighbor_p) {
    neighbor_p->timer->enable = false;
    _neighbors.erase(neighbor_p);
  }
  // 更新邻居过期计时器
  void update_timer(host_t host)  {
    auto neighbor_p = find_neighbor(host);
    if (neighbor_p != _neighbors.end()) update_neighbor_timer(neighbor_p);
  }
  // 根据neighbor指针,更新邻居过期计时器
  void update_neighbor_timer(neighbor_ptr neighbor_p)  {
    if (neighbor_p->timer) neighbor_p->timer->enable = false;
    neighbor_p->timer = _timeout.add_timer(WAIT_TIMEOUT, [=]()->void{

      #ifdef DEBUG
      log("timeout triggerd for " + neighbor_p->dest.to_string());
      #endif

      remove_neighbor(neighbor_p);
    });
  }
  // 发送路由表
  virtual void send_table(host_t host) = 0;
  // 更新路由表
  virtual void update_table(host_t host, table_t table) = 0;
  // 接收路由表
  virtual void receive_table(host_t host, table_t table) = 0;
  // 发送消息正文
  void send_message(host_t dest, host_t router, std::string message) {
    int header_len = sizeof(rip_header);

    rip_header rip_h;

    rip_h.type = MESSAGE;
    rip_h.dest = dest;

    std::string data = std::string((char *)&rip_h, header_len) + message;

    _udp.send(router.ip, router.port, data);
  }
  // 转发消息
  virtual void route_message(host_t dest, std::string message) = 0;
  // 接收消息
  virtual void receive_message(host_t host, std::string message) = 0;
  // 发送心跳信号
  virtual void send_heart_beat(host_t dest) {
    int tot_len = sizeof(rip_header);

    rip_header rip_h;

    rip_h.type = HEART;
    rip_h.dest = dest;

    std::string data((char *)&rip_h, tot_len);

    _udp.send(dest.ip, dest.port, data);
  }
  // 接收心跳信号
  virtual void receive_heart_beat(host_t host) {
    auto neighbor_p = find_neighbor(host);
    if (neighbor_p != _neighbors.end()) {
      update_neighbor_timer(neighbor_p);
    } else {
      add_neighbor(host);
    }
  }
  // 获取路由表
  const table_t & get_table() const { return _table; }
  // 序列化路由表
  virtual std::string stringify_table(table_t table) = 0;
  // 解析路由表消息
  virtual table_t parse_table(std::string table_str) = 0;

protected:
  host_t _localhost;  // 本机主机信息
  table_t _table;     // 路由表
  neighbor_list _neighbors;
  Udp _udp;           // UDP实体
  Timeout _timeout;   // 计时器工厂
  Timeout::timer_pcb_ptr _timer; // 计时器实体

};

};

#endif // __RIP__HPP__
