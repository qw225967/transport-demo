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

class UDPSender {
public:
  UDPSender(std::string ip, uint16_t port, uint64_t timer_ms);

  void run();
  void sender_test(uint16_t seq, uint32_t timestamp, const UDPEndpoint &ep);
private:
  void send_packet(TESTTPPacketPtr pkt, const UDPEndpoint &ep);

  void do_receive_from();
  void handle_receive_from(TESTTPPacketPtr pkt, const ErrorCode &ec, std::size_t bytes_recvd);

private:
  IOService     ios_;
  UDPSocketPrt  socket_;
  std::string   local_ip_;
  uint16_t      local_port_;
  uint64_t      timer_ms_;
  DeadlineTimer timer_;

  std::unordered_map<uint16_t, TESTTPPacketPtr> pkt_map_;
};

} // transport-demo

#endif //NACK_TEST_UDP_SERVER_H
