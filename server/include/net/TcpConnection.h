#pragma once

#include "net/Buffer.h"
#include "net/EventLoop.h"
#include "protocol/MessageDispatcher.h"

#include <functional>
#include <memory>
#include <string_view>

class TcpConnection {
public:
    TcpConnection(const TcpConnection&) = delete;
    auto operator=(TcpConnection&) -> TcpConnection& = delete;
    static auto create(EventLoop& loop, int fd, MessageDispatcher& dispatcher)
        -> std::unique_ptr<TcpConnection>;
    ~TcpConnection();
    void handleRead();
    void handleWrite();
    void close();
    void send(std::string_view data);
    auto fd() -> int {
        return fd_;
    }
    bool connected() const {
        return connected_;
    }

private:
    TcpConnection(EventLoop& loop, int fd, MessageDispatcher& dispatcher);

    EventLoop& loop_;
    int fd_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;
    bool connected_;
    MessageDispatcher& dispatcher_;
};
