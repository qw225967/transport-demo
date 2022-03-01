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
  static constexpr std::size_t RTCP_HEADER_SIZE_BYTES  = 4;
  static constexpr std::size_t NACK_ITEM_BYTES = 4;
  static constexpr std::size_t MAX_NACK_ITEM_NUM = 347;

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

  TESTTPPacketPtr Pack::packing_nack(const std::vector<uint16_t> &sequence_vector) {
    TESTTPPacketPtr packet = std::make_shared<TESTTPPacket>();

    TESTTCPHeader *header = reinterpret_cast<TESTTCPHeader *>(packet->mutable_buffer());
    header->type = 0;

    TESTTCPPayload *payload = reinterpret_cast<TESTTCPPayload *>(packet->mutable_buffer() + RTCP_HEADER_SIZE_BYTES);

    std::size_t nack_item_count = 0;
    auto iter = sequence_vector.begin();
    auto end = sequence_vector.end();
    while (iter != end) {
      FCI &fci = payload->nack.fci[nack_item_count];
      fci.pid = *iter++;
      fci.blp = 0;
      while (iter != end) {
        uint16_t shift = static_cast<uint16_t>(*iter - fci.pid - 1);
        if (shift <= 15) {
          fci.blp |= (1 << shift);
          ++iter;
        } else {
          break;
        }
      }
      fci.pid = htons(fci.pid);
      fci.blp = htons(fci.blp);
      nack_item_count++;
      if (nack_item_count == MAX_NACK_ITEM_NUM) {
        break;
      }
    }
    uint16_t length = static_cast<uint16_t>(RTCP_HEADER_SIZE_BYTES + NACK_ITEM_BYTES * nack_item_count);

    packet->mod_length(length);

    return packet;
  }

  bool Pack::unpacking_nack(TESTTPPacketPtr pkt, std::vector<uint16_t> &sequence_vector) {
    uint16_t len = pkt->length();

    // at least one nack item.
    if (len < RTCP_HEADER_SIZE_BYTES) {
      return false;
    }
    std::size_t nack_item = (len - (RTCP_HEADER_SIZE_BYTES)) / NACK_ITEM_BYTES;

    TESTTCPPayload *payload = reinterpret_cast<TESTTCPPayload *>(pkt->mutable_buffer() + RTCP_HEADER_SIZE_BYTES);

    for (std::size_t i = 0; i < nack_item; i++) {
      const FCI *fci = payload->nack.fci + i;
      uint16_t pid = fci->get_pid();
      uint16_t blp = fci->get_blp();

      sequence_vector.push_back(pid++);
      for (uint16_t bitmask = blp; bitmask != 0; bitmask >>= 1, ++pid) {
        if (bitmask & 1) {
          sequence_vector.push_back(pid);
        }
      }
    }

    return true;
  }

} // transport-demo