#pragma once

#include "net/EpollPoller.h"

#include <cstdint>
#include <functional>
#include <unordered_map>

class EventLoop {
public:
    using EventCallback = std::function<void(uint32_t)>;

    void add(int fd, uint32_t events, EventCallback callback);
    void remove(int fd);
    void run();
    void stop();

private:
    EpollPoller poller_;
    std::unordered_map<int, EventCallback> callbacks_;
    bool running_{false};
};