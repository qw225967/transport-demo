/*******************************************************
 * @author      : dog head
 * @date        : Created in 2022/3/1 3:49 下午
 * @mail        : qw225967@github.com
 * @project     : nack_test
 * @file        : udp_server.h
 * @description : TODO
 *******************************************************/


#ifndef NACK_TEST_UDP_SERVER_H
#define NACK_TEST_UDP_SERVER_H

#include <unordered_map>

#include "../test_tp.h"

namespace transportdemo {
typedef std::shared_ptr<TESTTPPacket> TESTTPPacketPtr;
class NackGenerator;
typedef std::shared_ptr<NackGenerator> NackGeneratorPtr;

class UDPSender {
public:
  UDPSender(std::string ip, uint16_t port, uint64_t timer_ms);

public:
  void run();

  void send_packet(TESTTPPacketPtr pkt, const UDPEndpoint &ep);

  void do_receive_from();
  void handle_receive_from(TESTTPPacketPtr pkt, const ErrorCode &ec, std::size_t bytes_recvd);
  void do_timer(bool first);
  void handle_crude_timer(const ErrorCode &ec);

private:
  IOService     ios_;
  UDPSocketPrt  socket_;
  std::string   local_ip_;
  uint16_t      local_port_;
  uint64_t      timer_ms_;
  DeadlineTimer timer_;


  std::unordered_map<uint16_t, UDPSocketPrt> pkt_map_;
  NackGeneratorPtr nackgen_;

  UDPEndpoint send_ep_;
};

} // transport-demo

#endif //NACK_TEST_UDP_SERVER_H
