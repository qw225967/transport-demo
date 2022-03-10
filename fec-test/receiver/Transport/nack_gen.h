/*******************************************************
 * @author      : dog head
 * @date        : Created in 2022/3/2 1:52 下午
 * @mail        : qw225967@github.com
 * @project     : receiver
 * @file        : nack_gen.h
 * @description : TODO
 *******************************************************/


#ifndef RECEIVER_NACK_GEN_H
#define RECEIVER_NACK_GEN_H

#include <map>
#include <memory>

//#include "../third_party/linux/boost/include/boost/date_time/posix_time/posix_time.hpp"
#include "/opt/homebrew/Cellar/boost/1.76.0/include/boost/date_time/posix_time/posix_time.hpp"

namespace transportdemo {
  class TESTTPPacket;
  typedef std::shared_ptr<TESTTPPacket> TESTTPPacketPtr;

  static constexpr uint16_t MaxValue = std::numeric_limits<uint16_t>::max();

  class NackGenerator {
  private:
    struct NackInfo
        {
          NackInfo() = default;
          explicit NackInfo(uint16_t seq, uint16_t sendAtSeq, uint64_t ms, uint8_t time)
          : seq(seq)
          , sendAtSeq(sendAtSeq)
          , sentAtMs(ms)
          , retries(time) {
          }

          uint16_t seq{ 0u };
          uint16_t sendAtSeq{ 0u };
          uint64_t sentAtMs{ 0u };
          uint8_t retries{ 0u };
        };

    enum class NackFilter
        {
      SEQ,
      TIME
        };

  public:
    explicit NackGenerator();
    ~NackGenerator();

    bool SeqLowerThan(const uint16_t lhs, const uint16_t rhs) {
      return ((rhs > lhs) && (rhs - lhs <= MaxValue / 2)) ||
      ((lhs > rhs) && (lhs - rhs > MaxValue / 2));
    }
    bool SeqHigherThan(const uint16_t lhs, const uint16_t rhs) {
      return ((lhs > rhs) && (lhs - rhs <= MaxValue / 2)) ||
      ((rhs > lhs) && (rhs - lhs > MaxValue / 2));
    }

    uint64_t GetCurrentStamp64() {
      boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
      boost::posix_time::time_duration time_from_epoch =
      boost::posix_time::microsec_clock::universal_time() - epoch;

      return time_from_epoch.total_microseconds()/1000;
    }

    bool ReceivePacket(TESTTPPacketPtr packet);
    size_t GetNackListLength() const {
      return this->nackList.size();
    }
    void UpdateRtt(uint32_t rtt) {
      this->rtt = rtt;
    }
    void Reset();
    std::vector<uint16_t> GetNackBatch();

  private:
    void AddPacketsToNackList(uint16_t seqStart, uint16_t seqEnd);

  private:
    // Others.
    std::map<uint16_t, NackInfo> nackList;
    bool started{ false };
    uint16_t lastSeq{ 0u }; // Seq number of last valid packet.
    uint32_t rtt{ 0u };     // Round trip time (ms).



  };


} // transportdemo

#endif //RECEIVER_NACK_GEN_H
