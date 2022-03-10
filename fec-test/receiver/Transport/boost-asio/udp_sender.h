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
#include <map>
#include <vector>
#include <unordered_map>

#include "../test_tp.h"

namespace transportdemo {
typedef std::shared_ptr<TESTTPPacket> TESTTPPacketPtr;
class NackGenerator;
typedef std::shared_ptr<NackGenerator> NackGeneratorPtr;
class FECGenerator;
typedef std::shared_ptr<FECGenerator> FECGeneratorPtr;

typedef std::vector<uint8_t> ByteArray;
struct Group{
  uint32_t group_id;
  std::map<uint32_t,std::shared_ptr<ByteArray> > packet_list; // key是seq-num，value是包的数据
};

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
  void fec_dencode_callback(uint64_t groupId, int16_t k, int16_t n, int16_t index, uint8_t *data,
                           size_t size);
  void collect_packet(TESTFECHeader *header, uint8_t* data,size_t size);
  void decode_fec_group(uint32_t groupID);

private:
  IOService     ios_;
  UDPSocketPrt  socket_;
  std::string   local_ip_;
  uint16_t      local_port_;
  uint64_t      timer_ms_;
  uint64_t      count_timer_ms_;
  DeadlineTimer timer_;


  std::unordered_map<uint16_t, UDPSocketPrt> pkt_map_;
  std::unordered_map<uint16_t, uint32_t> rtt_count_map_;
  uint16_t rtt_count_num_;
  uint32_t rtt_;

  NackGeneratorPtr nackgen_;

  UDPEndpoint send_ep_;

  FECGeneratorPtr fec_gen_;
  std::map<uint32_t,std::shared_ptr<Group> > groups_;
  std::unordered_map<uint32_t,bool> group_marks_;
};

} // transport-demo

#endif //NACK_TEST_UDP_SERVER_H
