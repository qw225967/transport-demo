/*******************************************************
 * @author      : dog head
 * @date        : Created in 2022/3/1 3:32 下午
 * @mail        : qw225967@github.com
 * @project     : fec_test
 * @file        : test-tp.h
 * @description : TODO
 *******************************************************/

#include "transport_fd.h"

#ifndef FEC_TEST_TEST_TP_H
#define FEC_TEST_TEST_TP_H
namespace transportdemo {
  static constexpr std::size_t UDP_PACKET_MAX_BYTES = 1400;

  struct TESTTCPHeader {
    uint16_t type;
    uint32_t length;
    uint16_t padding;

    uint16_t get_type() const { return ntohs(type); }
    uint32_t get_length() const { return ntohl(length); }
  };

  struct TESTTPHeader {
    uint16_t type;
    uint16_t sequence;
    uint32_t timestamp;

    uint16_t get_type() const { return ntohs(type); }
    uint16_t get_sequence() const { return ntohs(sequence); }
    uint32_t get_timestamp() const { return ntohl(timestamp); }
  };

  struct TESTFECHeader{
    uint16_t type; // 2B
    uint32_t length; // 4B
    uint16_t padding; // 2B
    int16_t fec_k; // 2B
    int16_t fec_n; // 2B
    int16_t fec_index; //2B
    uint32_t fec_group_id; // 4B
    uint16_t sequence; // 2B
    int16_t packet_size; // 2B
    int16_t last_packet_size; // 2B
    int16_t is_last_packet; // 2B

    uint16_t get_type() const { return ntohs(type); }
    uint32_t get_length() const { return ntohl(length); }
    uint16_t get_fec_k() const { return ntohs(fec_k); }
    uint16_t get_fec_n() const { return ntohs(fec_n); }
    uint16_t get_fec_index() const { return ntohs(fec_index); }
    uint32_t get_fec_group_id() const { return ntohl(fec_group_id); }
    uint16_t get_sequence() const { return ntohs(sequence); }
    uint16_t get_packet_size() const { return ntohs(packet_size); }
    uint16_t get_last_packet_size() const { return ntohs(last_packet_size); }
    uint16_t get_is_last_packet() const { return ntohs(is_last_packet); }
  };
  struct TESTFECPayload {
    TESTFECHeader header;
    uint8_t      buf[1300];
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
    struct {
      uint32_t num;
    }rtt;
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
#endif //FEC_TEST_TEST_TP_H
