#include "net/EventLoop.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstdint>
#include <fcntl.h>
#include <netinet/in.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

auto setNonBlocking(int fd) -> void {
    int flags = ::fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        throw std::runtime_error("fcntl(F_GETFL) failed");
    }

    if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        throw std::runtime_error("fcntl(F_SETFL) O_NONBLOCK failed");
    }
}
auto createListenSocket(uint16_t port) -> int {
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

auto main() -> int {
    auto console = spdlog::stdout_color_mt("server");

    spdlog::get("server")->info("Nebula Duel server starting...");

    int listen_fd = createListenSocket(9000);

    EventLoop loop;
    loop.add(listen_fd, EPOLLIN, [&loop, listen_fd](uint32_t revnets) {
        if (revnets & EPOLLIN) {
            int client_fd = ::accept4(listen_fd, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
            if (client_fd < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                    return;
                }
                throw std::runtime_error("accept4 failed");
            }
            loop.add(client_fd, EPOLLIN, [&loop, client_fd](uint32_t revents) {
                if (revents & EPOLLIN) {
                    char buf[128];
                    int count = ::read(client_fd, buf, 128);
                    if (count == 0) {
                        loop.remove(client_fd);
                        close(client_fd);
                        spdlog::get("server")->info("perr close connection");
                        return;
                    }
                    if (count < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                            return;
                        }
                        spdlog::get("server")->error("read %d falied", client_fd);
                        loop.remove(client_fd);
                        close(client_fd);
                    }
                    ::write(client_fd, buf, count);
                }
            });
        }
    });

    loop.run();

    return 0;
}
