#include "net/TcpConnection.h"

#include "protocol/MessageDispatcher.h"
#include "protocol/PacketCodec.h"

#include <cerrno>
#include <memory>
#include <spdlog/spdlog.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>

TcpConnection::TcpConnection(EventLoop& loop, int fd, MessageDispatcher& dispatcher)
    : loop_(loop), fd_(fd), connected_(true), dispatcher_(dispatcher) {}
TcpConnection::~TcpConnection() {}

auto TcpConnection::create(EventLoop& loop, int fd, MessageDispatcher& dispatcher)
    -> std::unique_ptr<TcpConnection> {
    return std::unique_ptr<TcpConnection>(new TcpConnection(loop, fd, dispatcher));
}

void TcpConnection::handleRead() {
    if (!connected_) {
        return;
    }
    char buf[4096];
    while (true) {
        ssize_t n = ::read(fd_, buf, sizeof(buf));
        if (n > 0) {

            inputBuffer_.append(buf, static_cast<size_t>(n));
            while (true) {
                Packet packet;

                auto result = PacketCodec::decode(inputBuffer_, packet);
                if (result == DecodeResult::NeedMore) {
                    break;
                }
                if (result == DecodeResult::Error) {
                    spdlog::get("server")->warn("invalid packet, closing fd={}", fd_);
                    close();
                    return;
                }

                spdlog::get("server")->info("packet msg_id = {}, seq_id={}, payload_size = {}",
                                            packet.msg_id, packet.seq_id, packet.payload.size());

                dispatcher_.dispatch(*this, packet);
            }
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
    if (fd_ >= 0) {
        loop_.remove(fd_);
        ::close(fd_);
        fd_ = -1;
    }
}

void TcpConnection::send(std::string_view data) {
    if (!connected_) {
        return;
    }

    outputBuffer_.append(data.data(), data.size());
    loop_.modify(fd_, EPOLLIN | EPOLLOUT);
}
