#include "net/Buffer.h"
#include "protocol/PacketCodec.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>
#include <string>

namespace {
void append_u32_be(std::string& out, uint32_t value) {
    char bytes[4] = {
        static_cast<char>((value >> 24) & 0xff),
        static_cast<char>((value >> 16) & 0xff),
        static_cast<char>((value >> 8) & 0xff),
        static_cast<char>(value & 0xff),
    };
    out.append(bytes, sizeof(bytes));
}
} // namespace

TEST(PacketCodecTest, EncodeThenDecode) {
    const auto encoded = PacketCodec::encode(1001, 42, "hello");

    Buffer buffer;
    buffer.append(encoded);

    Packet packet;
    const auto result = PacketCodec::decode(buffer, packet);

    EXPECT_EQ(result, DecodeResult::Ok);
    EXPECT_EQ(packet.msg_id, 1001U);
    EXPECT_EQ(packet.seq_id, 42U);
    EXPECT_EQ(packet.payload, "hello");
    EXPECT_EQ(buffer.readable_bytes(), 0U);
}

TEST(PacketCodecTest, HalfPacketReturnsNeedMoreAndKeepsBuffer) {
    const auto encoded = PacketCodec::encode(1002, 7, "split");

    Buffer buffer;
    buffer.append(encoded.data(), 5);

    Packet packet;
    const auto result = PacketCodec::decode(buffer, packet);

    EXPECT_EQ(result, DecodeResult::NeedMore);
    EXPECT_EQ(buffer.readable_bytes(), 5U);
}

TEST(PacketCodecTest, StickyPacketsDecodeOneByOne) {
    const auto first = PacketCodec::encode(1003, 1, "first");
    const auto second = PacketCodec::encode(1004, 2, "second");

    Buffer buffer;
    buffer.append(first);
    buffer.append(second);

    Packet packet;

    EXPECT_EQ(PacketCodec::decode(buffer, packet), DecodeResult::Ok);
    EXPECT_EQ(packet.msg_id, 1003U);
    EXPECT_EQ(packet.seq_id, 1U);
    EXPECT_EQ(packet.payload, "first");

    EXPECT_EQ(PacketCodec::decode(buffer, packet), DecodeResult::Ok);
    EXPECT_EQ(packet.msg_id, 1004U);
    EXPECT_EQ(packet.seq_id, 2U);
    EXPECT_EQ(packet.payload, "second");

    EXPECT_EQ(buffer.readable_bytes(), 0U);
}

TEST(PacketCodecTest, InvalidLengthReturnsError) {
    std::string invalid;
    append_u32_be(invalid, 4);

    Buffer buffer;
    buffer.append(invalid);

    Packet packet;
    const auto result = PacketCodec::decode(buffer, packet);

    EXPECT_EQ(result, DecodeResult::Error);
}

TEST(PacketCodecTest, EmptyPayloadIsValid) {
    const auto encoded = PacketCodec::encode(1005, 3, "");

    Buffer buffer;
    buffer.append(encoded);

    Packet packet;
    const auto result = PacketCodec::decode(buffer, packet);

    EXPECT_EQ(result, DecodeResult::Ok);
    EXPECT_EQ(packet.msg_id, 1005U);
    EXPECT_EQ(packet.seq_id, 3U);
    EXPECT_TRUE(packet.payload.empty());
}
