cmake_minimum_required(VERSION 3.20)
project(flexfec_test)

set(CMAKE_CXX_STANDARD 14)

set(WEBRTC_DIR /libwebrtc/libwebrtc)
set(WEBRTC_MODULES ${WEBRTC_DIR}/modules)

ADD_DEFINITIONS(-DWEBRTC_POSIX)

include_directories(
        ${WEBRTC_MODULES}/rtp_rtcp/include
        ${WEBRTC_MODULES}/rtp_rtcp/source
        libwebrtc/deps/abseil-cpp/abseil-cpp
        libwebrtc/libwebrtc/
)

aux_source_directory(libwebrtc/libwebrtc/modules/rtp_rtcp/source  RTP_RTCP_SOURCE_CC)
aux_source_directory(libwebrtc/libwebrtc/modules/rtp_rtcp/include  RTP_RTCP_INCLUDE_CC)
aux_source_directory(libwebrtc/libwebrtc/rtc_base  RTC_BASE_CC)
aux_source_directory(libwebrtc/libwebrtc/rtc_base/numerics  RTC_BASE_NUMERICS_CC)
aux_source_directory(libwebrtc/libwebrtc/system_wrappers/source  SYSTEM_WRAPPERS_CC)
aux_source_directory(libwebrtc/deps/abseil-cpp/abseil-cpp  ABSL_CC)
aux_source_directory(libwebrtc/deps/abseil-cpp/abseil-cpp/absl/types  ABSL_TYPE_CC)


add_executable(flexfec_test
        ${RTP_RTCP_INCLUDE_CC}
        ${RTP_RTCP_SOURCE_CC}
        ${RTC_BASE_CC}
        ${RTC_BASE_NUMERICS_CC}
        ${SYSTEM_WRAPPERS_CC}
        ${ABSL_CC}
        main.cpp
        mytest/test.cpp

        libwebrtc/deps/abseil-cpp/abseil-cpp/absl/types/bad_optional_access.cc
        libwebrtc/libwebrtc/api/video/color_space.cc
        libwebrtc/libwebrtc/api/video/hdr_metadata.cc
        libwebrtc/libwebrtc/api/rtp_parameters.cc
        libwebrtc/libwebrtc/rtc_base/strings/string_builder.cc
        libwebrtc/libwebrtc/rtc_base/synchronization/rw_lock_wrapper.cc
        libwebrtc/libwebrtc/rtc_base/synchronization/rw_lock_posix.cc
        libwebrtc/libwebrtc/rtc_base/synchronization/sequence_checker.cc
        libwebrtc/libwebrtc/api/transport/network_types.cc
        libwebrtc/libwebrtc/api/task_queue/task_queue_base.cc
        libwebrtc/libwebrtc/api/video/video_content_type.cc
        libwebrtc/libwebrtc/rtc_base/race_checker.cc
        libwebrtc/libwebrtc/modules/rtp_rtcp/source/forward_error_correction_85.cc
        libwebrtc/libwebrtc/modules/rtp_rtcp/source/rtp_packet_85.cc
        )
