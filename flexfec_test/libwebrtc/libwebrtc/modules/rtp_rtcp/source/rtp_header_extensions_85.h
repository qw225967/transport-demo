/*
 *  Copyright (c) 2016 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef MODULES_RTP_RTCP_SOURCE_RTP_HEADER_EXTENSIONS_85_H_
#define MODULES_RTP_RTCP_SOURCE_RTP_HEADER_EXTENSIONS_85_H_

#include <stddef.h>
#include <stdint.h>

#include <string>

#include "api/array_view_85.h"
#include "api/rtp_headers.h"
#include "api/video/color_space.h"
#include "api/video/video_content_type.h"
#include "api/video/video_rotation.h"
#include "api/video/video_timing.h"
#include "modules/rtp_rtcp/include/rtp_rtcp_defines.h"

namespace webrtc85 {

class AbsoluteSendTime {
 public:
  using value_type = uint32_t;
  static constexpr webrtc::RTPExtensionType kId = webrtc::kRtpExtensionAbsoluteSendTime;
  static constexpr uint8_t kValueSizeBytes = 3;
  static constexpr const char kUri[] =
      "http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time";

  static bool Parse(rtc::ArrayView<const uint8_t> data, uint32_t* time_24bits);
  static size_t ValueSize(uint32_t time_24bits) { return kValueSizeBytes; }
  static bool Write(rtc::ArrayView<uint8_t> data, uint32_t time_24bits);

  static constexpr uint32_t MsTo24Bits(int64_t time_ms) {
    return static_cast<uint32_t>(((time_ms << 18) + 500) / 1000) & 0x00FFFFFF;
  }
};

class AbsoluteCaptureTimeExtension {
 public:
  using value_type = webrtc::AbsoluteCaptureTime;
  static constexpr webrtc::RTPExtensionType kId = webrtc::kRtpExtensionAbsoluteCaptureTime;
  static constexpr uint8_t kValueSizeBytes = 16;
  static constexpr uint8_t kValueSizeBytesWithoutEstimatedCaptureClockOffset =
      8;
  static constexpr const char kUri[] =
      "http://www.webrtc.org/experiments/rtp-hdrext/abs-capture-time";

  static bool Parse(rtc::ArrayView<const uint8_t> data,
                    webrtc::AbsoluteCaptureTime* extension);
  static size_t ValueSize(const webrtc::AbsoluteCaptureTime& extension);
  static bool Write(rtc::ArrayView<uint8_t> data,
                    const webrtc::AbsoluteCaptureTime& extension);
};

class AudioLevel {
 public:
  static constexpr webrtc::RTPExtensionType kId = webrtc::kRtpExtensionAudioLevel;
  static constexpr uint8_t kValueSizeBytes = 1;
  static constexpr const char kUri[] =
      "urn:ietf:params:rtp-hdrext:ssrc-audio-level";

  static bool Parse(rtc::ArrayView<const uint8_t> data,
                    bool* voice_activity,
                    uint8_t* audio_level);
  static size_t ValueSize(bool voice_activity, uint8_t audio_level) {
    return kValueSizeBytes;
  }
  static bool Write(rtc::ArrayView<uint8_t> data,
                    bool voice_activity,
                    uint8_t audio_level);
};

class TransmissionOffset {
 public:
  using value_type = int32_t;
  static constexpr webrtc::RTPExtensionType kId = webrtc::kRtpExtensionTransmissionTimeOffset;
  static constexpr uint8_t kValueSizeBytes = 3;
  static constexpr const char kUri[] = "urn:ietf:params:rtp-hdrext:toffset";

  static bool Parse(rtc::ArrayView<const uint8_t> data, int32_t* rtp_time);
  static size_t ValueSize(int32_t rtp_time) { return kValueSizeBytes; }
  static bool Write(rtc::ArrayView<uint8_t> data, int32_t rtp_time);
};

class TransportSequenceNumber {
 public:
  using value_type = uint16_t;
  static constexpr webrtc::RTPExtensionType kId = webrtc::kRtpExtensionTransportSequenceNumber;
  static constexpr uint8_t kValueSizeBytes = 2;
  static constexpr const char kUri[] =
      "http://www.ietf.org/id/"
      "draft-holmer-rmcat-transport-wide-cc-extensions-01";
  static bool Parse(rtc::ArrayView<const uint8_t> data,
                    uint16_t* transport_sequence_number);
  static size_t ValueSize(uint16_t /*transport_sequence_number*/) {
    return kValueSizeBytes;
  }
  static bool Write(rtc::ArrayView<uint8_t> data,
                    uint16_t transport_sequence_number);
};

class TransportSequenceNumberV2 {
 public:
  static constexpr webrtc::RTPExtensionType kId =
      webrtc::kRtpExtensionTransportSequenceNumber02;
  static constexpr uint8_t kValueSizeBytes = 4;
  static constexpr uint8_t kValueSizeBytesWithoutFeedbackRequest = 2;
  static constexpr const char kUri[] =
      "http://www.webrtc.org/experiments/rtp-hdrext/transport-wide-cc-02";
  static bool Parse(rtc::ArrayView<const uint8_t> data,
                    uint16_t* transport_sequence_number,
                    absl::optional<webrtc::FeedbackRequest>* feedback_request);
  static size_t ValueSize(
      uint16_t /*transport_sequence_number*/,
      const absl::optional<webrtc::FeedbackRequest>& feedback_request) {
    return feedback_request ? kValueSizeBytes
                            : kValueSizeBytesWithoutFeedbackRequest;
  }
  static bool Write(rtc::ArrayView<uint8_t> data,
                    uint16_t transport_sequence_number,
                    const absl::optional<webrtc::FeedbackRequest>& feedback_request);

 private:
  static constexpr uint16_t kIncludeTimestampsBit = 1 << 15;
};

class VideoOrientation {
 public:
  using value_type = webrtc::VideoRotation;
  static constexpr webrtc::RTPExtensionType kId = webrtc::kRtpExtensionVideoRotation;
  static constexpr uint8_t kValueSizeBytes = 1;
  static constexpr const char kUri[] = "urn:3gpp:video-orientation";

  static bool Parse(rtc::ArrayView<const uint8_t> data, webrtc::VideoRotation* value);
  static size_t ValueSize(webrtc::VideoRotation) { return kValueSizeBytes; }
  static bool Write(rtc::ArrayView<uint8_t> data, webrtc::VideoRotation value);
  static bool Parse(rtc::ArrayView<const uint8_t> data, uint8_t* value);
  static size_t ValueSize(uint8_t value) { return kValueSizeBytes; }
  static bool Write(rtc::ArrayView<uint8_t> data, uint8_t value);
};

class PlayoutDelayLimits {
 public:
  using value_type = webrtc::PlayoutDelay;
  static constexpr webrtc::RTPExtensionType kId = webrtc::kRtpExtensionPlayoutDelay;
  static constexpr uint8_t kValueSizeBytes = 3;
  static constexpr const char kUri[] =
      "http://www.webrtc.org/experiments/rtp-hdrext/playout-delay";

  // Playout delay in milliseconds. A playout delay limit (min or max)
  // has 12 bits allocated. This allows a range of 0-4095 values which
  // translates to a range of 0-40950 in milliseconds.
  static constexpr int kGranularityMs = 10;
  // Maximum playout delay value in milliseconds.
  static constexpr int kMaxMs = 0xfff * kGranularityMs;  // 40950.

  static bool Parse(rtc::ArrayView<const uint8_t> data,
                    webrtc::PlayoutDelay* playout_delay);
  static size_t ValueSize(const webrtc::PlayoutDelay&) { return kValueSizeBytes; }
  static bool Write(rtc::ArrayView<uint8_t> data,
                    const webrtc::PlayoutDelay& playout_delay);
};

class VideoContentTypeExtension {
 public:
  using value_type = webrtc::VideoContentType;
  static constexpr webrtc::RTPExtensionType kId = webrtc::kRtpExtensionVideoContentType;
  static constexpr uint8_t kValueSizeBytes = 1;
  static constexpr const char kUri[] =
      "http://www.webrtc.org/experiments/rtp-hdrext/video-content-type";

  static bool Parse(rtc::ArrayView<const uint8_t> data,
                    webrtc::VideoContentType* content_type);
  static size_t ValueSize(webrtc::VideoContentType) { return kValueSizeBytes; }
  static bool Write(rtc::ArrayView<uint8_t> data,
                    webrtc::VideoContentType content_type);
};

class VideoTimingExtension {
 public:
  using value_type = webrtc::VideoSendTiming;
  static constexpr webrtc::RTPExtensionType kId = webrtc::kRtpExtensionVideoTiming;
  static constexpr uint8_t kValueSizeBytes = 13;
  static constexpr const char kUri[] =
      "http://www.webrtc.org/experiments/rtp-hdrext/video-timing";

  // Offsets of the fields in the RTP header extension, counting from the first
  // byte after the one-byte header.
  static constexpr uint8_t kFlagsOffset = 0;
  static constexpr uint8_t kEncodeStartDeltaOffset = 1;
  static constexpr uint8_t kEncodeFinishDeltaOffset = 3;
  static constexpr uint8_t kPacketizationFinishDeltaOffset = 5;
  static constexpr uint8_t kPacerExitDeltaOffset = 7;
  static constexpr uint8_t kNetworkTimestampDeltaOffset = 9;
  static constexpr uint8_t kNetwork2TimestampDeltaOffset = 11;

  static bool Parse(rtc::ArrayView<const uint8_t> data,
                    webrtc::VideoSendTiming* timing);
  static size_t ValueSize(const webrtc::VideoSendTiming&) { return kValueSizeBytes; }
  static bool Write(rtc::ArrayView<uint8_t> data,
                    const webrtc::VideoSendTiming& timing);

  static size_t ValueSize(uint16_t time_delta_ms, uint8_t idx) {
    return kValueSizeBytes;
  }
  // Writes only single time delta to position idx.
  static bool Write(rtc::ArrayView<uint8_t> data,
                    uint16_t time_delta_ms,
                    uint8_t offset);
};

class ColorSpaceExtension {
 public:
  using value_type = webrtc::ColorSpace;
  static constexpr webrtc::RTPExtensionType kId = webrtc::kRtpExtensionColorSpace;
  static constexpr uint8_t kValueSizeBytes = 28;
  static constexpr uint8_t kValueSizeBytesWithoutHdrMetadata = 4;
  static constexpr const char kUri[] =
      "http://www.webrtc.org/experiments/rtp-hdrext/color-space";

  static bool Parse(rtc::ArrayView<const uint8_t> data,
                    webrtc::ColorSpace* color_space);
  static size_t ValueSize(const webrtc::ColorSpace& color_space) {
    return color_space.hdr_metadata() ? kValueSizeBytes
                                      : kValueSizeBytesWithoutHdrMetadata;
  }
  static bool Write(rtc::ArrayView<uint8_t> data,
                    const webrtc::ColorSpace& color_space);

 private:
  static constexpr int kChromaticityDenominator = 50000;  // 0.00002 resolution.
  static constexpr int kLuminanceMaxDenominator = 1;      // 1 resolution.
  static constexpr int kLuminanceMinDenominator = 10000;  // 0.0001 resolution.

  static uint8_t CombineRangeAndChromaSiting(
      webrtc::ColorSpace::RangeID range,
      webrtc::ColorSpace::ChromaSiting chroma_siting_horizontal,
      webrtc::ColorSpace::ChromaSiting chroma_siting_vertical);
  static size_t ParseHdrMetadata(rtc::ArrayView<const uint8_t> data,
                                 webrtc::HdrMetadata* hdr_metadata);
  static size_t ParseChromaticity(const uint8_t* data,
                                  webrtc::HdrMasteringMetadata::Chromaticity* p);
  static size_t ParseLuminance(const uint8_t* data, float* f, int denominator);
  static size_t WriteHdrMetadata(rtc::ArrayView<uint8_t> data,
                                 const webrtc::HdrMetadata& hdr_metadata);
  static size_t WriteChromaticity(uint8_t* data,
                                  const webrtc::HdrMasteringMetadata::Chromaticity& p);
  static size_t WriteLuminance(uint8_t* data, float f, int denominator);
};

// Base extension class for RTP header extensions which are strings.
// Subclasses must defined kId and kUri static constexpr members.
class BaseRtpStringExtension {
 public:
  using value_type = std::string;
  // String RTP header extensions are limited to 16 bytes because it is the
  // maximum length that can be encoded with one-byte header extensions.
  static constexpr uint8_t kMaxValueSizeBytes = 16;

  static bool Parse(rtc::ArrayView<const uint8_t> data, std::string* str);
  static size_t ValueSize(const std::string& str) { return str.size(); }
  static bool Write(rtc::ArrayView<uint8_t> data, const std::string& str);
};

class RtpStreamId : public BaseRtpStringExtension {
 public:
  static constexpr webrtc::RTPExtensionType kId = webrtc::kRtpExtensionRtpStreamId;
  static constexpr const char kUri[] =
      "urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id";
};

class RepairedRtpStreamId : public BaseRtpStringExtension {
 public:
  static constexpr webrtc::RTPExtensionType kId = webrtc::kRtpExtensionRepairedRtpStreamId;
  static constexpr const char kUri[] =
      "urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id";
};

class RtpMid : public BaseRtpStringExtension {
 public:
  static constexpr webrtc::RTPExtensionType kId = webrtc::kRtpExtensionMid;
  static constexpr const char kUri[] = "urn:ietf:params:rtp-hdrext:sdes:mid";
};

//class InbandComfortNoiseExtension {
// public:
//  using value_type = absl::optional<uint8_t>;
//
//  static constexpr RTPExtensionType kId = kRtpExtensionInbandComfortNoise;
//  static constexpr uint8_t kValueSizeBytes = 1;
//  static constexpr const char kUri[] =
//      "http://www.webrtc.org/experiments/rtp-hdrext/inband-cn";
//
//  static bool Parse(rtc::ArrayView<const uint8_t> data,
//                    absl::optional<uint8_t>* level);
//  static size_t ValueSize(absl::optional<uint8_t> level) {
//    return kValueSizeBytes;
//  }
//  static bool Write(rtc::ArrayView<uint8_t> data,
//                    absl::optional<uint8_t> level);
//};

}  // namespace webrtc
#endif  // MODULES_RTP_RTCP_SOURCE_RTP_HEADER_EXTENSIONS_85_H_
