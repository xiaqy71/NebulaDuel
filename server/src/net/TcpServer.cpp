#include "net/TcpServer.h"

#include "common.pb.h"
#include "login.pb.h"
#include "net/TcpConnection.h"
#include "protocol/MessageId.h"
#include "protocol/PacketCodec.h"

#include <arpa/inet.h>
#include <cstdint>
#include <fcntl.h>
#include <netinet/in.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
TcpServer::TcpServer(EventLoop& loop, uint16_t port) : loop_(loop), port_(port) {
    dispatcher_.registerHandler(
        MessageId::kLoginRequest, [](TcpConnection& conn, const Packet& packet) {
            nebula::login::LoginRequest req;
            if (!req.ParseFromString(packet.payload)) {
                spdlog::get("server")->warn("parse LoginRequest failed");
                return;
            }

            spdlog::get("server")->info("login request username = {}", req.username());

            nebula::login::LoginResponse resp;
            resp.set_error_code(nebula::common::ERROR_CODE_OK);
            resp.set_message("login ok");
            resp.set_player_id(1);

            std::string payload;
            if (!resp.SerializeToString(&payload)) {
                spdlog::get("server")->warn("serialize LoginResponse failed");
                return;
            }

            conn.send(PacketCodec::encode(MessageId::kLoginResponse, packet.seq_id, payload));
        });
}
TcpServer::~TcpServer() = default;

void TcpServer::start() {
    listenFd_ = createListenSocket(port_);
    loop_.add(listenFd_, EPOLLIN, [this](uint32_t revents) {
        if (revents & EPOLLIN) {
            handleAccept();
        }
    });
}

void TcpServer::handleAccept() {
    while (true) {
        int client_fd = ::accept4(listenFd_, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                return;
            }
            throw std::runtime_error("accept4 failed");
        }
        auto conn = TcpConnection::create(loop_, client_fd, dispatcher_);
        connections_[client_fd] = std::move(conn);
        loop_.add(client_fd, EPOLLIN, [this, client_fd](uint32_t revents) {
            auto iter = connections_.find(client_fd);
            if (iter == connections_.end()) {
                return;
            }
            auto* conn = iter->second.get();
            if ((revents & EPOLLERR) || (revents & EPOLLHUP)) {
                conn->close();
                connections_.erase(client_fd);
                return;
            }
            if (revents & EPOLLIN) {
                conn->handleRead();
                if (!conn->connected()) {
                    connections_.erase(client_fd);
                    return;
                }
            }
            if (revents & EPOLLOUT) {
                conn->handleWrite();
                if (!conn->connected()) {
                    connections_.erase(client_fd);
                    return;
                }
            }
        });
    }
}

auto TcpServer::createListenSocket(uint16_t port) -> int {
    int listen_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        throw std::runtime_error("socket failed");
    }

    // 1) 地址复用（必须在 bind 前）
    int opt = 1;
    if (::setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        ::close(listen_fd);
        throw std::runtime_error("setsockopt SO_REUSEADDR failed");
    }

    // 2) 端口复用（可选：多进程/多实例负载分担）
#ifdef SO_REUSEPORT
    if (::setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        // 有些系统不支持，通常可以忽略失败
    }
#endif

    // 3) 非阻塞
    setNonBlocking(listen_fd);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (::bind(listen_fd, reinterpret_cast<sockaddr*>(&addr),
               static_cast<socklen_t>(sizeof(addr))) < 0) {
        ::close(listen_fd);
        throw std::runtime_error("bind failed");
    }

    // 4) 开始监听（别忘了）
    if (::listen(listen_fd, SOMAXCONN) < 0) {
        ::close(listen_fd);
        throw std::runtime_error("listen failed");
    }

    return listen_fd;
}

auto TcpServer::setNonBlocking(int fd) -> void {
    int flags = ::fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        throw std::runtime_error("fcntl(F_GETFL) failed");
    }

    if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        throw std::runtime_error("fcntl(F_SETFL) O_NONBLOCK failed");
    }
}
