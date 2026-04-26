#include "net/EventLoop.h"

#include <cstdint>

void EventLoop::add(int fd, uint32_t events, EventCallback callback) {
    callbacks_[fd] = std::move(callback);

    poller_.add(fd, events);
}

void EventLoop::remove(int fd) {
    poller_.remove(fd);
    callbacks_.erase(fd);
}

void EventLoop::run() {
    running_ = true;

    while (running_) {
        auto events = poller_.poll(1000);

        for (const auto& event : events) {
            const int fd = event.data.fd;
            auto iter = callbacks_.find(fd);
            if (iter != callbacks_.end()) {
                iter->second(event.events);
            }
        }
    }
}

void EventLoop::stop() {
    running_ = false;
}