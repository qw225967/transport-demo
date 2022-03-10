/*******************************************************
 * @author      : dog head
 * @date        : Created in 2022/3/1 3:51 下午
 * @mail        : qw225967@github.com
 * @project     : fec_test
 * @file        : udp_sender.cpp
 * @description : TODO
 *******************************************************/

#include <vector>

#include "udp_sender.h"
#include "pack.h"
#include "FEC/fec_gen.h"

#include <string>
#include <ostream>
#include <iomanip>


#define FEC_K 4
#define FEC_N 8
#define FEC_SIZE 10


namespace transportdemo {
  std::string bytes_to_hex(const uint8_t *buf, std::size_t len, std::size_t num_per_line) {
    if (buf == NULL || len == 0 || num_per_line == 0) {
      return std::string();
    }
    std::ostringstream oss;
    for (std::size_t i = 0; i < len; i++) {
      oss << std::right << std::setw(3) << std::hex << static_cast<int>(buf[i]);
      if ((i+1) % num_per_line == 0) {
        oss << '\n';
      }
    }
    if (len % num_per_line != 0) {
      oss << '\n';
    }
    return oss.str();
  }

  UDPSender::UDPSender(std::string ip, uint16_t port, uint64_t timer_ms)
  : local_ip_(ip)
  , local_port_(port)
  , timer_ms_(timer_ms)
  , seq_(1)
  , timer_(ios_, PosixTime::milliseconds(static_cast<int64_t>(timer_ms_))){
    socket_ = std::make_shared<UDPSocket>(ios_);
    boost::asio::ip::address send_addr = boost::asio::ip::address::from_string("192.168.26.23");
//    boost::asio::ip::address send_addr = boost::asio::ip::address::from_string("127.0.0.1");
    UDPEndpoint send_endpoint(send_addr,8001);
    send_ep_ = send_endpoint;

    fec_gen_ = std::make_shared<FECGenerator>(FEC_K, FEC_N, FEC_SIZE);
  }

  void UDPSender::run() {
    UDPEndpoint local_endpoint(Address::from_string(local_ip_.c_str()), local_port_);
    socket_->open(local_endpoint.protocol());
    socket_->set_option(UDPSocket::reuse_address(true));
    socket_->bind(local_endpoint);
    do_receive_from();

    do_timer(true);

    try {
      ios_.run();
      std::cout << "UDPSender run success" << std::endl;
    } catch (std::exception &e) {
      std::cout << "UDPSender run err" << std::endl;
    }

  }

  void UDPSender::sender_test(uint16_t seq, uint32_t timestamp, const UDPEndpoint &ep) {
    auto pkt = Pack::packing_packet(seq, timestamp);
    pkt_map_[seq] = pkt;

//    std::cout << bytes_to_hex(pkt->mutable_buffer(), pkt->length(), 8) << std::endl;
    fec_gen_->Encode(pkt->mutable_buffer(), pkt->length(), std::bind(&UDPSender::fec_encode_callback, this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2,
                                                                     std::placeholders::_3,
                                                                     std::placeholders::_4,
                                                                     std::placeholders::_5,
                                                                     std::placeholders::_6));

    send_packet(pkt, ep);
  }

  void UDPSender::send_packet(TESTTPPacketPtr pkt, const UDPEndpoint &ep) {
    socket_->send_to(boost::asio::buffer(pkt->mutable_buffer(), pkt->length()), ep);
  }


  void UDPSender::do_receive_from() {
    TESTTPPacketPtr pkt = std::make_shared<TESTTPPacket>();
    socket_->async_receive_from(boost::asio::buffer(pkt->mutable_buffer(), pkt->capacity()),
                                        pkt->mutable_endpoint(),
                                        std::bind(&UDPSender::handle_receive_from,
                                                  this,
                                                  pkt,
                                                  std::placeholders::_1,
                                                  std::placeholders::_2));
  }

  void UDPSender::handle_receive_from(TESTTPPacketPtr pkt, const ErrorCode &ec, std::size_t bytes_recvd) {
    TESTTCPHeader *header = reinterpret_cast<TESTTCPHeader *>(pkt->mutable_buffer());

    if (header->get_type() == 12) {
      TESTTCPPayload * rtt = reinterpret_cast<TESTTCPPayload *>(pkt->mutable_buffer()+8);
      auto rtt_pack = Pack::rtt_packing(rtt->rtt.num);
      send_packet(rtt_pack, pkt->mutable_endpoint());
      do_receive_from();
      return;
    }


    std::vector<uint16_t> seqs;
    Pack::unpacking_nack(pkt, seqs);
    std::cout << "recv packet nack size:" << seqs.size() << std::endl;

    for (auto seq : seqs) {
      auto ite = pkt_map_.find(seq);
      if (ite != pkt_map_.end()) {
        send_packet(ite->second, pkt->mutable_endpoint());
      }
    }

    do_receive_from();
  }

  void UDPSender::do_timer(bool first) {
    if (!first) {
      timer_.expires_at(timer_.expires_at() +
      PosixTime::milliseconds(static_cast<int64_t>(timer_ms_)));
    }
    timer_.async_wait(std::bind(&UDPSender::handle_crude_timer, this, std::placeholders::_1));
  }
  void UDPSender::handle_crude_timer(const ErrorCode &ec) {
    uint32_t now = (uint32_t)GetCurrentStamp64();
    sender_test(seq_, now, send_ep_);
    seq_++;

    do_timer(false);
  }

  void UDPSender::fec_encode_callback(uint64_t groupId, int16_t k, int16_t n, int16_t index, uint8_t *data,
                           size_t size) {



    if(groupId % 2 == 0){
      if(index < FEC_K){
        //printf("client drop [%lld][%d]\n",groupId,index);
        return;
      }
    }
    else{
      if(index > FEC_K){
        //printf("client drop [%lld][%d]\n",groupId,index);
        return;
      }
    }
//    std::cout << bytes_to_hex(data, size, 8) << std::endl;

    auto pkt = Pack::fec_packing(groupId, k, n, index, data, size);

//    std::cout << bytes_to_hex(pkt->mutable_buffer(), pkt->length(), 8) << std::endl;
    send_packet(pkt, send_ep_);

//    TESTFECHeader* header = reinterpret_cast<TESTFECHeader*>(pkt->mutable_buffer());
//    TESTFECPayload* payload = reinterpret_cast<TESTFECPayload*>(pkt->mutable_buffer());
//    fec_gen_->Decode(header, payload->buf, std::bind(&UDPSender::fec_dencode_callback, this,
//                                                     std::placeholders::_1,
//                                                     std::placeholders::_2,
//                                                     std::placeholders::_3,
//                                                     std::placeholders::_4,
//                                                     std::placeholders::_5,
//                                                     std::placeholders::_6));

  }

  void UDPSender::fec_dencode_callback(uint64_t groupId, int16_t k, int16_t n, int16_t index, uint8_t *data,
                                       size_t size) {

//    std::cout << bytes_to_hex(data, size, 8) << std::endl;
    TESTTPHeader *header = reinterpret_cast<TESTTPHeader *>(data);
    std::cout << "fec_encode_callback:"<< header->get_sequence() << std::endl;

  }

}