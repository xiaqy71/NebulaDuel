cmake_minimum_required(VERSION 3.20)

project(NebulaDuelServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(spdlog REQUIRED)
find_package(GTest REQUIRED)
find_package(Protobuf REQUIRED)
find_package(absl REQUIRED)

add_executable(nebula_server
    src/main.cpp
    src/net/EpollPoller.cpp
    src/net/EventLoop.cpp
    src/net/TcpServer.cpp
    src/net/TcpConnection.cpp
    src/net/Buffer.cpp
    src/protocol/PacketCodec.cpp
    generated/common.pb.cc
    generated/login.pb.cc
    src/protocol/MessageDispatcher.cpp
)

target_include_directories(nebula_server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/generated
)

target_link_libraries(nebula_server
    PRIVATE
        spdlog::spdlog
        protobuf::libprotobuf
        absl::log_internal_check_op
        absl::log_internal_message
        absl::log_internal_conditions
        absl::strings
        absl::base

)

target_compile_options(nebula_server
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

enable_testing()

add_executable(packet_codec_test
    tests/PacketCodecTest.cpp
    src/net/Buffer.cpp
    src/protocol/PacketCodec.cpp
)

target_include_directories(packet_codec_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(packet_codec_test
    PRIVATE
        GTest::gtest_main
)

target_compile_options(packet_codec_test
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

include(GoogleTest)
gtest_discover_tests(packet_codec_test)
