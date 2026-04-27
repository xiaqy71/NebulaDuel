#include "net/EventLoop.h"

#include <cstdint>

void EventLoop::add(int fd, uint32_t events, EventCallback callback) {

    poller_.add(fd, events);
    callbacks_[fd] = std::move(callback);
}

void EventLoop::remove(int fd) {
    auto iter = callbacks_.find(fd);
    if (iter == callbacks_.end()) {
        return;
    }
    poller_.remove(fd);
    callbacks_.erase(fd);
}

void EventLoop::modify(int fd, uint32_t events) {
    poller_.modify(fd, events);
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
