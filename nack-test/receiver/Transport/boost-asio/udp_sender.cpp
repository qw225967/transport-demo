/*******************************************************
 * @author      : dog head
 * @date        : Created in 2022/3/1 3:51 下午
 * @mail        : qw225967@github.com
 * @project     : nack_test
 * @file        : udp_sender.cpp
 * @description : TODO
 *******************************************************/

#include "udp_sender.h"
#include "../test_tp.h"
#include "../nack_gen.h"
#include "../pack.h"

namespace transportdemo {
  UDPSender::UDPSender(std::string ip, uint16_t port, uint64_t timer_ms)
  : local_ip_(ip)
  , local_port_(port)
  , timer_ms_(timer_ms)
  , timer_(ios_, PosixTime::milliseconds(static_cast<int64_t>(timer_ms_))){
    socket_ = std::make_shared<UDPSocket>(ios_);
    nackgen_ = std::make_shared<NackGenerator>();
    boost::asio::ip::address send_addr = boost::asio::ip::address::from_string("127.0.0.1");
    UDPEndpoint send_endpoint(send_addr,8000);
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
    send_ep_ = pkt->mutable_endpoint();

    TESTTPHeader *header = reinterpret_cast<TESTTPHeader *>(pkt->mutable_buffer());


    uint32_t send_time = header->get_timestamp();
    uint32_t recv_time = (uint32_t)nackgen_->GetCurrentStamp64();
    auto rtt = recv_time - send_time;
//    std::cout << "recv packet seq:" << header->get_sequence() << ", rtt:"<< rtt << std::endl;
//    if (header->get_sequence() %34 == 0) {
//      std::cout << "recv packet seq:" << header->get_sequence() << ", rtt:"<< rtt << std::endl;
//    }

    nackgen_->UpdateRtt(rtt);
    nackgen_->ReceivePacket(pkt);

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
//    std::cout << "do_timer" << std::endl;

    auto seqs = nackgen_->GetNackBatch();
    if (seqs.size() > 0)
      std::cout << "do_timer seqs size:" << seqs.size() << std::endl;
    auto nack = Pack::packing_nack(seqs);
    if (!seqs.empty())
      this->send_packet(nack, send_ep_);

    do_timer(false);
  }

}