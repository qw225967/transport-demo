/*******************************************************
 * @author      : dog head
 * @date        : Created in 2022/3/1 3:44 下午
 * @mail        : qw225967@github.com
 * @project     : fec_test
 * @file        : pack.h
 * @description : TODO
 *******************************************************/


#ifndef FEC_TEST_PACK_H
#define FEC_TEST_PACK_H
#include <memory>
#include <vector>

namespace transportdemo {
  class TESTTPPacket;
  typedef std::shared_ptr<TESTTPPacket> TESTTPPacketPtr;

  class Pack{
  public:
    static TESTTPPacketPtr packing_packet(uint16_t sequence, uint32_t timestamp);
    static TESTTPPacketPtr packing_nack(const std::vector<uint16_t> &sequence_vector);
    static bool unpacking_nack(TESTTPPacketPtr pkt, std::vector<uint16_t> &sequence_vector);
    static TESTTPPacketPtr rtt_packing(uint16_t num);
  };
} // transport-demo

#endif //FEC_TEST_PACK_H
