#include "net/EpollPoller.h"

#include <cstdint>
#include <stdexcept>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>

EpollPoller::EpollPoller() : epollFd_(epoll_create1(EPOLL_CLOEXEC)), events_(kInitMaxEvents) {
    if (epollFd_ < 0) {
        throw std::runtime_error("failed to create epoll fd");
    }
}

EpollPoller::~EpollPoller() {
    if (epollFd_ >= 0) {
        close(epollFd_);
    }
}

void EpollPoller::add(int fd, uint32_t events) {
    epoll_event event{};
    event.events = events;
    event.data.fd = fd;

    if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &event) < 0) {
        throw std::runtime_error("failed to add fd to epoll");
    }
}

void EpollPoller::modify(int fd, uint32_t events) {
    epoll_event event{};
    event.events = events;
    event.data.fd = fd;

    if (epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &event) < 0) {
        throw std::runtime_error("failed to modify fd in epoll");
    }
}

void EpollPoller::remove(int fd) {
    if (epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, nullptr) < 0) {
        throw std::runtime_error("failed to remove fd from epoll");
    }
}

auto EpollPoller::poll(int timeoutMs) -> std::vector<epoll_event> {
    const int count =
        epoll_wait(epollFd_, events_.data(), static_cast<int>(events_.size()), timeoutMs);
    if (count < 0) {
        throw std::runtime_error("epoll_wait failed");
    }

    return {events_.begin(), events_.begin() + count};
}