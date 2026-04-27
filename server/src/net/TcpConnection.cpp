#include "net/TcpConnection.h"

#include <cerrno>
#include <memory>
#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>

TcpConnection::TcpConnection(EventLoop& loop, int fd) : loop_(loop), fd_(fd), connected_(true) {}
TcpConnection::~TcpConnection() {}

auto TcpConnection::create(EventLoop& loop, int fd) -> std::unique_ptr<TcpConnection> {
    return std::unique_ptr<TcpConnection>(new TcpConnection(loop, fd));
}

void TcpConnection::setClosecallback(CloseCallback cb) {
    closeCallback_ = std::move(cb);
}

void TcpConnection::handleRead() {
    if (!connected_) {
        return;
    }
    char buf[4096];
    while (true) {
        ssize_t n = ::read(fd_, buf, sizeof(buf));
        if (n > 0) {
            send(std::string_view(buf, static_cast<size_t>(n)));
            continue;
        }
        if (n == 0) {
            close();
            return;
        }
        if (errno == EINTR) {
            continue;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        }
        close();
        return;
    }
}
void TcpConnection::handleWrite() {
    if (!connected_) {
        return;
    }
    while (outputBuffer_.readable_bytes() > 0) {
        ssize_t n = ::write(fd_, outputBuffer_.peek(), outputBuffer_.readable_bytes());
        if (n > 0) {
            outputBuffer_.retrieve(static_cast<size_t>(n));
            continue;
        }
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return;
            }

            close();
            return;
        }
    }
    loop_.modify(fd_, EPOLLIN);
}

void TcpConnection::close() {
    if (!connected_) {
        return;
    }
    connected_ = false;
    const int old_fd = fd_;
    if (fd_ >= 0) {
        loop_.remove(fd_);
        ::close(fd_);
        fd_ = -1;
    }
    if (closeCallback_) {
        closeCallback_(old_fd);
    }
}

void TcpConnection::send(std::string_view data) {
    if (!connected_) {
        return;
    }

    outputBuffer_.append(data.data(), data.size());
    loop_.modify(fd_, EPOLLIN | EPOLLOUT);
}
