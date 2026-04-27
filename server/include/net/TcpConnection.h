#pragma once

#include "net/Buffer.h"
#include "net/EventLoop.h"

#include <functional>
#include <memory>
#include <string_view>

class TcpConnection {
public:
    using CloseCallback = std::function<void(int)>;
    void setClosecallback(CloseCallback cb);
    TcpConnection(const TcpConnection&) = delete;
    auto operator=(TcpConnection&) -> TcpConnection& = delete;
    static auto create(EventLoop& loop, int fd) -> std::unique_ptr<TcpConnection>;
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
    TcpConnection(EventLoop& loop, int fd);

    EventLoop& loop_;
    int fd_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;
    bool connected_;

    CloseCallback closeCallback_;
};
