#pragma once

#include "net/Buffer.h"

#include <cstdint>
#include <string>
#include <string_view>

struct Packet {
    uint32_t msg_id{};
    uint32_t seq_id{};
    std::string payload;
};

enum class DecodeResult {
    Ok,
    NeedMore,
    Error,
};

class PacketCodec {
public:
    static constexpr uint32_t kHeaderSize = 12;
    static constexpr uint32_t kMaxPacketSize = 1024 * 1024;

    static std::string encode(uint32_t msg_id, uint32_t seq_id, std::string_view payload);
    static DecodeResult decode(Buffer& buffer, Packet& packet);
};
