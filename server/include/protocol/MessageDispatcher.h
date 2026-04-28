#pragma once

#include "protocol/PacketCodec.h"

#include <cstdint>
#include <functional>
#include <unordered_map>

class TcpConnection;

class MessageDispatcher {
public:
    using Handler = std::function<void(TcpConnection& conn, const Packet& packet)>;
    void registerHandler(uint32_t msg_id, Handler handler);
    void dispatch(TcpConnection& conn, const Packet& packet) const;

private:
    std::unordered_map<uint32_t, Handler> handlers_;
};
