#include "net/TcpConnection.h"
#include "protocol/MessageDispatcher.h"

#include <cstdint>
#include <spdlog/spdlog.h>

void MessageDispatcher::registerHandler(uint32_t msg_id, Handler handler) {
    handlers_[msg_id] = std::move(handler);
}

void MessageDispatcher::dispatch(TcpConnection& conn, const Packet& packet) const {
    auto iter = handlers_.find(packet.msg_id);
    if (iter == handlers_.end()) {
        spdlog::get("server")->warn("unknown msg_id = {}", packet.msg_id);
        return;
    }
    iter->second(conn, packet);
}
