/*******************************************************
 * @author      : dog head
 * @date        : Created in 2022/3/2 1:52 下午
 * @mail        : qw225967@github.com
 * @project     : receiver
 * @file        : nack_gen.cpp
 * @description : TODO
 *******************************************************/

#include "nack_gen.h"
#include "pack.h"
#include "test_tp.h"

namespace transportdemo {
  /* Static. */

  constexpr size_t MaxPacketAge{ 10000u };
  constexpr size_t MaxNackPackets{ 1000u };
  constexpr uint32_t DefaultRtt{ 100u };
  //	constexpr uint8_t MaxNackRetries{ 10u };
  //	constexpr uint64_t TimerInterval{ 40u };

  // =================== nack test ===================
  constexpr uint8_t MaxNackRetries{ 20u };
  constexpr uint64_t TimerInterval{ 20u };
  // =================== nack test ===================

  /* Instance methods. */

  NackGenerator::NackGenerator() : rtt(DefaultRtt) {
  }

  NackGenerator::~NackGenerator() {
  }

  // Returns true if this is a found nacked packet. False otherwise.
  bool NackGenerator::ReceivePacket(TESTTPPacketPtr packet) {
    TESTTPHeader* header = reinterpret_cast<TESTTPHeader *>(packet->mutable_buffer());
    uint16_t seq    = header->get_sequence();

    if (!this->started) {
      this->started = true;
      this->lastSeq = seq;

      return false;
    }

    // Obviously never nacked, so ignore.
    if (seq == this->lastSeq)
      return false;

    // May be an out of order packet, or already handled retransmitted packet,
    // or a retransmitted packet.
    if (SeqLowerThan(seq, this->lastSeq)) {
      auto it = this->nackList.find(seq);

      // It was a nacked packet.
      if (it != this->nackList.end()) {
        this->nackList.erase(it);

        return true;
      }

      return false;
    }

    AddPacketsToNackList(this->lastSeq + 1, seq);
    this->lastSeq = seq;

    return false;
  }

  void NackGenerator::AddPacketsToNackList(uint16_t seqStart, uint16_t seqEnd) {

    // Remove old packets.
    auto it = this->nackList.lower_bound(seqEnd - MaxPacketAge);

    this->nackList.erase(this->nackList.begin(), it);

    // If the nack list is too large, remove packets from the nack list until
    // the latest first packet of a keyframe. If the list is still too large,
    // clear it and request a keyframe.
    uint16_t numNewNacks = seqEnd - seqStart;

    if (this->nackList.size() + numNewNacks > MaxNackPackets)
    {
      if (this->nackList.size() + numNewNacks > MaxNackPackets)
      {
        this->nackList.clear();

        return;
      }
    }

    for (uint16_t seq = seqStart; seq != seqEnd; ++seq)
    {
      if (this->nackList.find(seq) == this->nackList.end())
        this->nackList.emplace(std::make_pair(seq, NackInfo{ seq, seq, 0, 0 }));
    }
  }

  std::vector<uint16_t> NackGenerator::GetNackBatch() {
    uint64_t nowMs = GetCurrentStamp64();
    NackFilter filter = NackFilter::TIME;
    std::vector<uint16_t> nackBatch;

    auto it = this->nackList.begin();

    while (it != this->nackList.end()) {
      NackInfo& nackInfo = it->second;
      uint16_t seq       = nackInfo.seq;

      // clang-format off
//      if ( filter == NackFilter::SEQ &&
//           nackInfo.sentAtMs == 0 &&
//           ( nackInfo.sendAtSeq == this->lastSeq ||
//             SeqHigherThan(this->lastSeq, nackInfo.sendAtSeq))
//          ) {
//        nackBatch.emplace_back(seq);
//        nackInfo.retries++;
//        nackInfo.sentAtMs = nowMs;
//
//        if (nackInfo.retries >= MaxNackRetries) {
//          it = this->nackList.erase(it);
//        } else {
//          ++it;
//        }
//
//        continue;
//        }

      if (filter == NackFilter::TIME && nowMs - nackInfo.sentAtMs >= this->rtt) {
        nackBatch.emplace_back(seq);
        nackInfo.retries++;
        auto oldMs = nackInfo.sentAtMs;
        if (oldMs == 0)
          oldMs = nowMs;
        nackInfo.sentAtMs = nowMs;
        std::cout << "retry seq:" << seq << ", times:" << nackInfo.retries << ", interval:" << nowMs-oldMs << std::endl;
        if (nackInfo.retries >= MaxNackRetries) {
          it = this->nackList.erase(it);
        } else {
          ++it;
        }

        continue;
        }

      ++it;
    }
    return nackBatch;
  }

  void NackGenerator::Reset() {
    this->nackList.clear();
    this->started = false;
    this->lastSeq = 0u;
  }
} // namespace transport-demo
