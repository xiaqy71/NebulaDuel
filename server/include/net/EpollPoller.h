#pragma once

#include <cstdint>
#include <sys/epoll.h>
#include <vector>

class EpollPoller {
public:
    EpollPoller();
    ~EpollPoller();

    EpollPoller(const EpollPoller&) = delete;
    auto operator=(const EpollPoller&) = delete;

    void add(int fd, uint32_t events);
    void modify(int fd, uint32_t events);
    void remove(int fd);

    auto poll(int timeoutMs) -> std::vector<epoll_event>;

private:
    static const int kInitMaxEvents = 64;
    int epollFd_{-1};
    std::vector<epoll_event> events_;
};