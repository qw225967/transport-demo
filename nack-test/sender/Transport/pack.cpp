/*******************************************************
 * @author      : dog head
 * @date        : Created in 2022/3/1 4:42 下午
 * @mail        : qw225967@github.com
 * @project     : nack_test
 * @file        : pack.cpp
 * @description : TODO
 *******************************************************/

#include "pack.h"
#include "test_tp.h"

namespace transportdemo {
  static constexpr std::size_t RTP_HEADER_SIZE_BYTES  = 8;

  static void packing_header(TESTTPPacketPtr packet, uint16_t sequence, uint32_t timestamp) {
    TESTTPHeader *header = reinterpret_cast<TESTTPHeader *>(packet->mutable_buffer());
    header->sequence = htons(sequence);
    header->timestamp = htonl(timestamp);
    header->padding = 0;
  }

  TESTTPPacketPtr Pack::packing_packet(uint16_t sequence, uint32_t timestamp){
    TESTTPPacketPtr packet;
    packet = std::make_shared<TESTTPPacket>();
    packing_header(packet, sequence, timestamp);
    TESTTPPayload *payload = nullptr;
    payload = reinterpret_cast<TESTTPPayload *>(packet->mutable_buffer() + RTP_HEADER_SIZE_BYTES);
    memset(payload->buf, 0x0, sizeof(uint8_t));
    packet->mod_length(RTP_HEADER_SIZE_BYTES + 1300);

    return packet;
  }

} // transport-demo