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
#include "../FEC/fec_gen.h"

#define FEC_K 8
#define FEC_N 8
#define FEC_SIZE 10
#define MAX_GROUPS 4

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
  , count_timer_ms_(0)
  , rtt_count_num_(1)
  , rtt_(1)
  , timer_(ios_, PosixTime::milliseconds(static_cast<int64_t>(timer_ms_))){
    socket_ = std::make_shared<UDPSocket>(ios_);
    nackgen_ = std::make_shared<NackGenerator>();
    boost::asio::ip::address send_addr = boost::asio::ip::address::from_string("127.0.0.1");
    UDPEndpoint send_endpoint(send_addr,8000);
    send_ep_ = send_endpoint;

    fec_gen_= std::make_shared<FECGenerator>(FEC_K, FEC_N, FEC_SIZE);
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
    pkt->mod_length(bytes_recvd);


    // 解析类型
    TESTTPHeader *header = reinterpret_cast<TESTTPHeader *>(pkt->mutable_buffer());
    
    // nack
    if (header->get_type() == 12) {
      TESTTCPPayload *payload = reinterpret_cast<TESTTCPPayload *>(pkt->mutable_buffer() + 8);
      auto num = payload->rtt.num;
      auto iter = rtt_count_map_.find(num);
      if (iter != rtt_count_map_.end()) {
        uint32_t recv_time = (uint32_t)nackgen_->GetCurrentStamp64();
        rtt_ = ((recv_time - iter->second) + rtt_)/2;
        rtt_count_map_.erase(iter);
      }
      do_receive_from();
      return;
    }
    
    // fec
    if (header->get_type() == 13) {
//      std::cout << bytes_to_hex(pkt->mutable_buffer(), pkt->length(), 8) << std::endl;

      TESTFECPayload *payload = reinterpret_cast<TESTFECPayload *>(pkt->mutable_buffer());
      TESTFECHeader *header_fec = reinterpret_cast<TESTFECHeader *>(pkt->mutable_buffer());
//      collect_packet(header_fec, payload->buf, header_fec->get_length());

      // 接到fec数据解码
      fec_gen_->Decode(header_fec, payload->buf, std::bind(&UDPSender::fec_dencode_callback, this,
                                                 std::placeholders::_1,
                                                 std::placeholders::_2,
                                                 std::placeholders::_3,
                                                 std::placeholders::_4,
                                                 std::placeholders::_5,
                                                 std::placeholders::_6));

      do_receive_from();
      return;
    }

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
    if (count_timer_ms_ % 80 == 0) {
      rtt_count_map_[rtt_count_num_] = (uint32_t)nackgen_->GetCurrentStamp64();
      auto rtt_packet = Pack::rtt_packing(rtt_count_num_);
      this->send_packet(rtt_packet, send_ep_);
      rtt_count_num_++;
    }

    if (count_timer_ms_ % 400 == 0) {
      nackgen_->UpdateRtt(rtt_);
//      std::cout << "rtt :" << rtt_ << std::endl;
    }

    auto seqs = nackgen_->GetNackBatch();
//    if (seqs.size() > 0)
//      std::cout << "do_timer seqs size:" << seqs.size() << std::endl;
    auto nack = Pack::packing_nack(seqs);
    if (!seqs.empty())
      this->send_packet(nack, send_ep_);

    count_timer_ms_ += timer_ms_;
    do_timer(false);
  }

  void UDPSender::fec_dencode_callback(uint64_t groupId, int16_t k, int16_t n, int16_t index, uint8_t *data,
                           size_t size) {


    TESTTPPacketPtr packet;
    packet = std::make_shared<TESTTPPacket>();
    TESTTPHeader *header_data = reinterpret_cast<TESTTPHeader *>(data);
    TESTTPHeader *header_pkt = reinterpret_cast<TESTTPHeader *>(packet->mutable_buffer());
    header_pkt->type = header_data->type;
    header_pkt->sequence = header_data->sequence;
    header_pkt->timestamp = header_data->timestamp;

    TESTTPPayload *payload_data  = reinterpret_cast<TESTTPPayload *>(data);
    TESTTPPayload *payload_pkt  = reinterpret_cast<TESTTPPayload *>(packet->mutable_buffer());
    memcpy(payload_pkt->buf, payload_data->buf, size);

    packet->mod_length(8 + 1300);



//    std::cout << bytes_to_hex(data, size, 8) << std::endl;
    std::cout << "fec_encode_callback:"<< header_pkt->get_sequence() << std::endl;
    nackgen_->ReceivePacket(packet);
  }

  void UDPSender::collect_packet(TESTFECHeader *header, uint8_t* data, size_t size){
    // 先把fec头部解析出来，下面会有用
    uint32_t seqNum = header->get_fec_index();
    uint32_t groupId = header->get_fec_group_id();
    uint16_t index = header->get_fec_index();

    std::shared_ptr<Group> group= nullptr;

    // 太老的包，直接丢弃
    if(groups_.size() >= MAX_GROUPS){
      group = groups_.begin()->second;
      if(groupId < group->group_id){ // 过期
        return;
      }
    }

    // 如果某一个group已经解码了，但是后面又受到这个group中的一个包，那么直接丢弃
    if(group_marks_.find(groupId) != group_marks_.end()){
      if(group_marks_[groupId]){
        return ;
      }
    }

    // 检查新包所属的group是否存在，不存在就创建
    if(groups_.find(groupId) == groups_.end()){
      std::shared_ptr<Group> g(new Group);
      g->group_id = groupId;
      groups_[groupId] = g;
      group = g;
    } else {
      group = groups_.find(groupId)->second;
    }
    // 保存数据
    std::shared_ptr< ByteArray> tempData(new ByteArray(data,data+size));
    group->packet_list[seqNum] = tempData;

    // 这个包所属的group还没有被处理
    group_marks_[groupId] = false;

    //  检查，是否有太老的group
    uint64_t groupIdDiff = 0;
    groupIdDiff = groups_.rbegin()->second->group_id - groups_.begin()->second->group_id;

    while(groupIdDiff > MAX_GROUPS){
      std::shared_ptr<Group> g = groups_.begin()->second;
      if(g->packet_list.size() >= FEC_K) {
        decode_fec_group(g->group_id);
      } else {
//        printf("drop[%lld]\n",groupId);
      }

      groups_.erase(groups_.begin());
      group_marks_[g->group_id] = true;

      groupIdDiff = groups_.rbegin()->second->group_id - groups_.begin()->second->group_id;
    }

    // 再检查所有的group，如果某一个group的包的数量达到了 fec_n 那么可以进行fec解码了
    // 如果还不够 fec_n个包，那还可以继续缓存，因为乱序、丢包等问题可能导致数据迟到
    std::map<uint32_t,std::shared_ptr<Group> >::iterator it = groups_.begin();
    while (it != groups_.end()) {
      std::shared_ptr<Group> g = it->second;
      if(g->packet_list.size() == FEC_N){
        decode_fec_group(g->group_id);
        it = groups_.erase(it);
        group_marks_[g->group_id] = true;
      }
      else{
        ++it;
      }
    }
  }
  // group test
  void UDPSender::decode_fec_group(uint32_t groupID){
    if(groups_.find(groupID) == groups_.end()){
      return;
    }
    std::shared_ptr<Group> group = groups_.find(groupID)->second;

    do{
      if(group->packet_list.size() < FEC_N){
        //printf("lost packet[%lld]\n",group->group_id);
      }

      if(group->packet_list.size() < FEC_K){
//        printf("can not fec decode\n");
        group->packet_list.clear();
        break;
      }
      int count = 0;

      std::map<uint32_t,std::shared_ptr<ByteArray> >::iterator it;

      for(it = group->packet_list.begin();it != group->packet_list.end(); ++it){
        std::shared_ptr<ByteArray> rawData = it->second;
        TESTFECHeader *header = reinterpret_cast<TESTFECHeader *>((char*)&(*rawData)[0]);
        TESTFECPayload *payload = reinterpret_cast<TESTFECPayload *>((char*)&(*rawData)[0]);

        ++count;

        if(count > FEC_K){
          continue;
        }

        fec_gen_->Decode(header, payload->buf, std::bind(&UDPSender::fec_dencode_callback, this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2,
                                                             std::placeholders::_3,
                                                             std::placeholders::_4,
                                                             std::placeholders::_5,
                                                             std::placeholders::_6));
      }
    }while(0);
  }

}
