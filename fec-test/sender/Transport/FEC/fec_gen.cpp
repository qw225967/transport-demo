/*******************************************************
 * @author      : dog head
 * @date        : Created in 2022/3/8 4:39 下午
 * @mail        : qw225967@github.com
 * @project     : nack_test
 * @file        : fec_gen.cpp
 * @description : TODO
 *******************************************************/

#include "fec_gen.h"

extern "C"{
#include "fec.h"
}


#define MAX_FEC_PACKET_SIZE 1300

namespace transportdemo {
  FECGenerator::FECGenerator(int k,int n,int packetSize){
    fec_k = k;
    fec_n = n;
    packet_count = 0;
    raw_packet_count = 0;
    packet_size = packetSize;
    last_packet_size = packetSize;

    last_group_id = 0;
    fec = fec_new(k,n);
    assert(fec);

    for(int i = 0; i < kMaxN; ++i){
      input_data[i] = new uint8_t[MAX_FEC_PACKET_SIZE];
      output_data[i] = new uint8_t[MAX_FEC_PACKET_SIZE];
      used_marks[i] = false;
    }

  }
  FECGenerator::~FECGenerator(){
    for(int i = 0; i < kMaxN; ++i){
      delete[] input_data[i];
      delete[] output_data[i];
    }
    fec_free(fec);
  }

  void FECGenerator::Encode(uint8_t *data, size_t size, std::function<void(uint64_t groupId, int16_t k, int16_t n, int16_t index, uint8_t *data,
      size_t size)> callback){
    // 如果是原始数据，那么先复制到数组中，然后返回，因为要凑齐k个数据才能fec编码
    if(packet_count < fec_k){
      memcpy(input_data[packet_count],data,size);

      fec_encode(fec,(void**)input_data,output_data[packet_count],packet_count,size);

      callback(last_group_id, fec_k, fec_n, packet_count, output_data[packet_count], size);

      ++packet_count;
      ++raw_packet_count;
    }

    if(packet_count < fec_k)
      return ;

    for(int i = fec_k; i < fec_n; ++i){
      fec_encode(fec,(void**)input_data,output_data[i],i,packet_size);
      callback(last_group_id,fec_k,fec_n,i,output_data[i],packet_size);
      ++packet_count;
    }

    ++last_group_id;

    // 每次处理完一个group，就清理一次，准备下一个group
    Clear();
  }

  void FECGenerator::Decode(TESTFECHeader *header, uint8_t *decoded_data, std::function<void(uint64_t groupId, int16_t k, int16_t n, int16_t index, uint8_t *data,
      size_t size)> callback){

    last_packet_size = header->last_packet_size;

    if(header->fec_group_id != last_group_id){
      Clear();
      last_group_id = header->fec_group_id;
    }

    // 如果index指向的位置还没有数据，表示是第一次接收到这块数据，那么把数据放进buffer中
    // 另外，如果这块数据是原始数据，那么还需要回调给上层
    if(used_marks[header->fec_index] == false){

      memcpy(input_data[header->fec_index],decoded_data,header->length);
      used_marks[header->fec_index] = true;

      // 属于原始包，那么返回它；冗余包不能返回
      if(header->fec_index < fec_k){
        callback(last_group_id, header->fec_k, header->fec_n, header->fec_index, decoded_data, header->length);
        ++raw_packet_count;
      }

      ++packet_count;
    }

    if(packet_count < fec_k){
      return ;
    }

    // 原始数据没有丢失，不需要恢复
    if(raw_packet_count == fec_k){
      return;
    }

    // 开始恢复

    uint8_t* recovery[kMaxN] = {0};
    int recoveryIndex[kMaxN] = {0};

    bool tempMarks[kMaxN] = {false};
    memcpy(tempMarks,used_marks,sizeof(bool) * kMaxN);

    // 先把原始数据放到指定位置
    for(int i = 0; i < fec_k; ++i){
      if(tempMarks[i] == true){
        recovery[i] = input_data[i];
        recoveryIndex[i] = i;
      }
    }

    // 把冗余数据放到指定位置
    for(int i = 0; i < fec_k; ++i){
      if(tempMarks[i]){
        continue;
      }

      for(int j = fec_k; j < fec_n; ++j){
        if(tempMarks[j] == true){
          tempMarks[j] = false;
          tempMarks[i] = true;
          recovery[i] = input_data[j];
          recoveryIndex[i] = j;
          break;
        }
      }
    }

    for(int i = 0; i < fec_k; ++i){
      if(recovery[i] == NULL || tempMarks[i] == false){
        return;
      }
    }

    // 数据恢复
    fec_decode(fec,(void**)recovery,recoveryIndex,packet_size);//recovery

    for(int i =0; i < fec_k;++i){
      if(recoveryIndex[i]== i){
        continue;
      }
      callback(last_group_id,fec_k,fec_n,i,recovery[i],packet_size);

      ++raw_packet_count;
      ++packet_count;
    }
    /*for(int i = 0; i < fec_k; ++i){
        if(used_marks[i]){
            continue;
        }

        callback(last_group_id,fec_k,fec_n,i,input_data[i],packet_size);

        ++raw_packet_count;
        ++packet_count;

    }*/
  }

  void FECGenerator::Clear(){
    packet_count = 0;
    raw_packet_count = 0;
    memset(used_marks,0,sizeof(bool) * kMaxN);
  }

} // transport-demo