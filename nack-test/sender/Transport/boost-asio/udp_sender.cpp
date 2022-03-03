/*******************************************************
 * @author      : dog head
 * @date        : Created in 2022/3/1 3:51 下午
 * @mail        : qw225967@github.com
 * @project     : nack_test
 * @file        : udp_sender.cpp
 * @description : TODO
 *******************************************************/

#include <vector>

#include "udp_sender.h"
#include "pack.h"

namespace transportdemo {
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
//    if (seq_ % 30 == 0) {
//      for (int i=seq_; i<=seq_+4;i++) {
//        auto pkt = Pack::packing_packet(i, now);
//        pkt_map_[i] = pkt;
//      }
//
//
//      seq_+=4;
//    }


    do_timer(false);
  }
}