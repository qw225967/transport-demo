/*******************************************************
 * @author      : dog head
 * @date        : Created in 2022/3/1 3:49 下午
 * @mail        : qw225967@github.com
 * @project     : fec_test
 * @file        : udp_server.h
 * @description : TODO
 *******************************************************/


#ifndef FEC_TEST_UDP_SERVER_H
#define FEC_TEST_UDP_SERVER_H

#include <unordered_map>

#include "../test_tp.h"

namespace transportdemo {
typedef std::shared_ptr<TESTTPPacket> TESTTPPacketPtr;
class FECGenerator;
typedef std::shared_ptr<FECGenerator> FECGeneratorPtr;

class UDPSender {
public:
  UDPSender(std::string ip, uint16_t port, uint64_t timer_ms);

  void run();
  void sender_test(uint16_t seq, uint32_t timestamp, const UDPEndpoint &ep);
private:
  void send_packet(TESTTPPacketPtr pkt, const UDPEndpoint &ep);

  void do_receive_from();
  void handle_receive_from(TESTTPPacketPtr pkt, const ErrorCode &ec, std::size_t bytes_recvd);
  void do_timer(bool first);
  void handle_crude_timer(const ErrorCode &ec);
  void fec_encode_callback(uint64_t groupId, int16_t k, int16_t n, int16_t index, uint8_t *data,
                           size_t size);
  void fec_dencode_callback(uint64_t groupId, int16_t k, int16_t n, int16_t index, uint8_t *data,
                                       size_t size);

  uint64_t GetCurrentStamp64() {
    boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
    boost::posix_time::time_duration time_from_epoch =
        boost::posix_time::microsec_clock::universal_time() - epoch;

    return time_from_epoch.total_microseconds()/1000;
  }
private:
  IOService     ios_;
  UDPSocketPrt  socket_;
  std::string   local_ip_;
  uint16_t      local_port_;
  uint64_t      timer_ms_;
  DeadlineTimer timer_;

  std::unordered_map<uint16_t, TESTTPPacketPtr> pkt_map_;

  UDPEndpoint send_ep_;
  uint16_t seq_;

  FECGeneratorPtr fec_gen_;
};

} // transport-demo

#endif //FEC_TEST_UDP_SERVER_H
