/*******************************************************
 * @author      : dog head
 * @date        : Created in 2022/3/8 4:39 下午
 * @mail        : qw225967@github.com
 * @project     : fec_test
 * @file        : fec_gen.h
 * @description : TODO
 *******************************************************/


#ifndef FEC_GEN_H
#define FEC_GEN_H

#include <assert.h>
#include <vector>
#include <map>
#include <memory>

#include "packet.h"

namespace transportdemo {
  // FEC编码回调
  typedef void (*fec_encode_callback)(uint64_t groupId, int16_t k, int16_t n, int16_t index, uint8_t *data,
                                      size_t size);

  // FEC解码回调
  typedef void (*fec_decode_callback)(uint64_t groupId, int16_t k, int16_t n, int16_t index, uint8_t *data,
                                      size_t size);

  class FECGenerator {
  private:
    enum {
      kMaxN = 128, // k或者n的最大值，简单设置为128，没必要设置得太大

    };
  public:
    FECGenerator(int k, int n, int packetSize);

    ~FECGenerator();

    void Encode(uint8_t *data, size_t size, fec_encode_callback callback);

    // 如果packet == NULL，那么表示开始利用冗余块去恢复原始块
    void Decode(FecPacketHead *head, uint8_t *data, size_t size, fec_decode_callback callback);


  private:
    void Clear();


    void *fec;

    // k和n
    int16_t fec_k;
    int16_t fec_n;

    // 普通的包的大小（不包括FEC头部，只是数据部分），由构造函数传进来的参数决定
    int16_t packet_size;

    // 最后一个包的小（不包括FEC头部，只是数据部分）
    // 对于一个文件或者一段数据来说，它的长度一般不可能是FEC包的整数倍，因此
    // 最后一个包的长度可能稍微小一点
    int16_t last_packet_size;

    // fec包的数量：原始包+冗余包
    int16_t packet_count;
    // fec原始包的数量
    int16_t raw_packet_count;

    uint8_t *input_data[kMaxN];
    uint8_t *output_data[kMaxN];

    // 解码时使用，用于标记一个group中的某一个包是否存在
    bool used_marks[kMaxN];

    // 当前group的id
    // k个原始包加上n-k个冗余包构成一个group
    uint64_t last_group_id;
  };

} // transport-demo

#endif //FEC_GEN_H
