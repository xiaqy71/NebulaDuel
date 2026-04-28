#include "protocol/PacketCodec.h"

#include <cstdint>
#include <cstring>
#include <netinet/in.h>
#include <stdexcept>

namespace {
uint32_t read_u32_be(const char* data) {
    uint32_t value = 0;
    std::memcpy(&value, data, sizeof(value));
    return ntohl(value);
}

void append_u32_be(std::string& out, uint32_t value) {
    const uint32_t network_value = htonl(value);
    const char* data = reinterpret_cast<const char*>(&network_value);
    out.append(data, sizeof(network_value));
}
} // namespace

std::string PacketCodec::encode(uint32_t msg_id, uint32_t seq_id, std::string_view payload) {
    if (payload.size() > kMaxPacketSize - 8) {
        throw std::runtime_error("payload too large");
    }

    const auto length = static_cast<uint32_t>(8 + payload.size());

    std::string out;
    out.reserve(kHeaderSize + payload.size());

    append_u32_be(out, length);
    append_u32_be(out, msg_id);
    append_u32_be(out, seq_id);
    out.append(payload.data(), payload.size());

    return out;
}

DecodeResult PacketCodec::decode(Buffer& buffer, Packet& packet) {
    if (buffer.readable_bytes() < sizeof(uint32_t)) {
        return DecodeResult::NeedMore;
    }
    const char* data = buffer.peek();
    const uint32_t length = read_u32_be(data);
    if (length < 8 || length > kMaxPacketSize) {
        return DecodeResult::Error;
    }
    const std::size_t total_size = sizeof(uint32_t) + length;
    if (buffer.readable_bytes() < total_size) {
        return DecodeResult::NeedMore;
    }

    packet.msg_id = read_u32_be(data + 4);
    packet.seq_id = read_u32_be(data + 8);

    buffer.retrieve(kHeaderSize);
    packet.payload = buffer.retrieve_as_string(length - 8);

    return DecodeResult::Ok;
}
