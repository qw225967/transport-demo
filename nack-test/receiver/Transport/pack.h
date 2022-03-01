/*******************************************************
 * @author      : dog head
 * @date        : Created in 2022/3/1 3:44 下午
 * @mail        : qw225967@github.com
 * @project     : nack_test
 * @file        : pack.h
 * @description : TODO
 *******************************************************/


#ifndef NACK_TEST_PACK_H
#define NACK_TEST_PACK_H
#include <memory>

namespace transportdemo {
  class TESTTPPacket;
  typedef std::shared_ptr<TESTTPPacket> TESTTPPacketPtr;

  class Pack{
  public:
    static TESTTPPacketPtr packing_packet(uint16_t sequence, uint32_t timestamp);
  };
} // transport-demo

#endif //NACK_TEST_PACK_H
