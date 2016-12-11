#include "udp.hpp"
#include <sys/socket.h>  //for socket ofcourse
#include <unistd.h>
#include <stdlib.h> //for exit(0);
#include <errno.h> //For errno - the error number
#include <arpa/inet.h>

// #define DEBUG
#define DATAGRAM_MAX_LEN 4096

#ifdef DEBUG

#include <iostream>

#endif

rip::Udp::Udp(port_t source_port): _source_port(source_port) {
  _finish = false;
  _socketfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (_socketfd < 0) {
    throw std::runtime_error("create socket failed");
  }
  _recive_thread = std::thread(&Udp::receive_loop, this);
}

rip::Udp::~Udp() {
  _finish = true;
  _recive_thread.join();
  close(_socketfd);
}

int rip::Udp::send(ip_t dest_ip, port_t dest_port, std::string data) {

  #ifdef DEBUG

  std::cout << "===== udp send datagram =====" << std::endl;
  std::cout << "remote client: " << stringify_ip(dest_ip) << ":" << dest_port << std::endl;
  std::cout << "local port: " << source_port << std::endl;
  std::cout << "data: " << data << std::endl;
  std::cout << "=============================" << std::endl;

  #endif

  // 缓存数据防止在被发送前析构
  _send_buffer = data;

  // 定义目的主机
  struct sockaddr_in dest_addr;

  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(dest_port); // 目的端口
  dest_addr.sin_addr.s_addr = dest_ip; // 目的ip

  socklen_t dest_addr_len = sizeof(dest_addr);

  return sendto(_socketfd, _send_buffer.c_str(), _send_buffer.size(), 0, (struct sockaddr *) &dest_addr, dest_addr_len);
}


  /**
   * [add_listener 添加监听器方法]
   * @param  port [监听端口]
   * @param  func [监听函数，测试中用的是lambda形式]
   * @return      [listener_ptr , 监听器的指针]
   */
rip::Udp::listener_ptr rip::Udp::add_listener(rip::Udp::listener func) {
  _listeners.push_front(func);
  return _listeners.begin();
}

  /**
   * [remove_listener 移除监听器方法]
   * @param  port [监听端口]
   * @param  ptr  [监听器的指针]
   * @return      [布尔值，移除操作处理结果]
   */
bool rip::Udp::remove_listener(rip::Udp::listener_ptr ptr) {
  _listeners.erase(ptr);
  return true;
}


void rip::Udp::receive_loop() {

  sockaddr_in source_addr, dest_addr;

  // 监听主机信息
  source_addr.sin_family = AF_INET;
  source_addr.sin_port = htons(_source_port);
  source_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 监听任何端口数据

  socklen_t source_addr_len = sizeof(source_addr),
            dest_addr_len   = sizeof(dest_addr);

  if (bind(_socketfd, (struct sockaddr *) &source_addr, source_addr_len) < 0) {
      throw std::runtime_error("bind failed");
  }

  int tot_len;
  char datagram[DATAGRAM_MAX_LEN];

  while (!_finish) {
    if ((tot_len = recvfrom (_socketfd, datagram, sizeof(datagram), MSG_DONTWAIT, (struct sockaddr *) &dest_addr, &dest_addr_len)) != -1) {

      // 源主机信息
      ip_t dest_ip = dest_addr.sin_addr.s_addr;
      port_t dest_port = ntohs(dest_addr.sin_port);

      std::string data = std::string(datagram, tot_len);

      for (auto it = _listeners.begin(); it != _listeners.end(); ++it) {
        (*it)(dest_ip, dest_port, data);
      }

    }
  }

}

rip::Udp::ip_t rip::Udp::parse_ip(std::string ip) {
  return inet_addr(ip.c_str());
}

std::string rip::Udp::stringify_ip(ip_t ip) {
  auto addr = in_addr();
  addr.s_addr = ip;
  return std::string(inet_ntoa(addr));
}
