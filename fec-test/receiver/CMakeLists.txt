cmake_minimum_required(VERSION 3.20)
project(nack_test)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_BOOST_DIR  ${CMAKE_SOURCE_DIR}/third_party/linux/boost)
#set(CMAKE_BOOST_DIR  /opt/homebrew/Cellar/boost/1.76.0)

message(STATUS ${CMAKE_BUILD_TYPE})
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

include_directories(
        ${CMAKE_BOOST_DIR}/include
        ${CMAKE_SOURCE_DIR}/Transport
        ${CMAKE_SOURCE_DIR}/Transport/boost-asio
)

link_directories(
        ${CMAKE_BOOST_DIR}/lib
)

add_executable(
        receiver
        main.cpp
        Transport/pack.cpp
        Transport/boost-asio/udp_sender.cpp
        Transport/nack_gen.cpp
        Transport/FEC/fec.c
        Transport/FEC/fec_gen.cpp
)
target_link_libraries(
        receiver
        boost_thread
        boost_system
        boost_filesystem
        boost_program_options
        boost_date_time
        pthread
)
