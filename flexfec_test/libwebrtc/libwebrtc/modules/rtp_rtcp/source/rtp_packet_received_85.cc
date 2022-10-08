/*
 *  Copyright (c) 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "modules/rtp_rtcp/source/rtp_packet_received_85.h"

#include <stddef.h>

#include <cstdint>
#include <vector>

#include "modules/rtp_rtcp/source/rtp_header_extensions_85.h"
#include "rtc_base/numerics/safe_conversions.h"

namespace webrtc85 {

RtpPacketReceived::RtpPacketReceived() = default;
RtpPacketReceived::RtpPacketReceived(const ExtensionManager* extensions)
    : RtpPacket(extensions) {}
RtpPacketReceived::RtpPacketReceived(const RtpPacketReceived& packet) = default;
RtpPacketReceived::RtpPacketReceived(RtpPacketReceived&& packet) = default;

RtpPacketReceived& RtpPacketReceived::operator=(
    const RtpPacketReceived& packet) = default;
RtpPacketReceived& RtpPacketReceived::operator=(RtpPacketReceived&& packet) =
    default;

RtpPacketReceived::~RtpPacketReceived() {}

void RtpPacketReceived::GetHeader(webrtc::RTPHeader* header) const {
  header->markerBit = Marker();
  header->payloadType = PayloadType();
  header->sequenceNumber = SequenceNumber();
  header->timestamp = Timestamp();
  header->ssrc = Ssrc();
  std::vector<uint32_t> csrcs = Csrcs();
  header->numCSRCs = rtc::dchecked_cast<uint8_t>(csrcs.size());
  for (size_t i = 0; i < csrcs.size(); ++i) {
    header->arrOfCSRCs[i] = csrcs[i];
  }
  header->paddingLength = padding_size();
  header->headerLength = headers_size();
  header->payload_type_frequency = payload_type_frequency();
  header->extension.hasTransmissionTimeOffset =
      GetExtension<webrtc85::TransmissionOffset>(
          &header->extension.transmissionTimeOffset);
  header->extension.hasAbsoluteSendTime =
      GetExtension<webrtc85::AbsoluteSendTime>(&header->extension.absoluteSendTime);
  header->extension.absolute_capture_time =
      GetExtension<webrtc85::AbsoluteCaptureTimeExtension>();
  header->extension.hasTransportSequenceNumber =
      GetExtension<webrtc85::TransportSequenceNumberV2>(
          &header->extension.transportSequenceNumber,
          &header->extension.feedback_request) ||
          GetExtension<webrtc85::TransportSequenceNumber>(
          &header->extension.transportSequenceNumber);
  header->extension.hasAudioLevel = GetExtension<webrtc85::AudioLevel>(
      &header->extension.voiceActivity, &header->extension.audioLevel);
  header->extension.hasVideoRotation =
      GetExtension<webrtc85::VideoOrientation>(&header->extension.videoRotation);
  header->extension.hasVideoContentType =
      GetExtension<webrtc85::VideoContentTypeExtension>(
          &header->extension.videoContentType);
  header->extension.has_video_timing =
      GetExtension<webrtc85::VideoTimingExtension>(&header->extension.video_timing);
  GetExtension<webrtc85::RtpStreamId>(&header->extension.stream_id);
  GetExtension<webrtc85::RepairedRtpStreamId>(&header->extension.repaired_stream_id);
  GetExtension<webrtc85::RtpMid>(&header->extension.mid);
  GetExtension<webrtc85::PlayoutDelayLimits>(&header->extension.playout_delay);
  header->extension.color_space = GetExtension<webrtc85::ColorSpaceExtension>();
}

}  // namespace webrtc
