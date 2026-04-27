#pragma once

#include "net/EventLoop.h"

#include <cstdint>
#include <memory>
#include <unordered_map>

class TcpConnection;
class TcpServer {
public:
    TcpServer(EventLoop& loop, uint16_t port);
    ~TcpServer();

    TcpServer(const TcpServer&) = delete;
    auto operator=(const TcpServer&) -> TcpServer& = delete;

    void start();
    void handleAccept();

private:
    auto createListenSocket(uint16_t port) -> int;
    auto setNonBlocking(int fd) -> void;

private:
    EventLoop& loop_;
    int listenFd_{-1};
    uint16_t port_;
    std::unordered_map<int, std::unique_ptr<TcpConnection>> connections_;
};
