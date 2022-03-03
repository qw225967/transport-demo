/*******************************************************
 * @author      : dog head
 * @date        : Created in 2022/3/1 3:32 下午
 * @mail        : qw225967@github.com
 * @project     : nack_test
 * @file        : test-tp.h
 * @description : TODO
 *******************************************************/
#include "../transport_fd.h"

#ifndef NACK_TEST_TEST_TP_H
#define NACK_TEST_TEST_TP_H
namespace transportdemo {
  static constexpr std::size_t UDP_PACKET_MAX_BYTES = 1400;

  struct TESTTCPHeader {
    uint32_t type;
    uint32_t length;

    uint32_t get_type() const { return ntohl(type); }
    uint32_t get_length() const { return ntohl(length); }
  };

  struct TESTTPHeader {
    uint16_t sequence;
    uint32_t timestamp;
    uint16_t padding;

    uint16_t get_sequence() const { return ntohs(sequence); }
    uint32_t get_timestamp() const { return ntohl(timestamp); }
  };

  struct TESTTPPayload {
    TESTTPHeader header;
    uint8_t      buf[1300];
  };

  struct FCI {
    uint16_t pid;
    uint16_t blp;

    uint16_t get_pid() const { return ntohs(pid); }
    uint16_t get_blp() const { return ntohs(blp); }

  }; // struct _FCI

  union TESTTCPPayload {
    struct {
      FCI fci[1];
    }nack;
  };

  class TESTTPPacket {
  public:
    TESTTPPacket() : capacity_(UDP_PACKET_MAX_BYTES) {
      buf_ = new uint8_t[UDP_PACKET_MAX_BYTES];
    }
    ~TESTTPPacket() {
      delete [] buf_;
    }

  public:
    uint8_t *mutable_buffer() { return buf_; }
    std::size_t capacity() const { return capacity_; }
    UDPEndpoint &mutable_endpoint() { return endpoint_; }
    std::size_t length() const { return length_; }
    void mod_length(std::size_t l) { length_ = l; }
  public:
    const std::size_t capacity_;
    uint8_t           *buf_;
    UDPEndpoint       endpoint_;
    std::size_t       length_;
  };
} // transport-demo
#endif //NACK_TEST_TEST_TP_H
