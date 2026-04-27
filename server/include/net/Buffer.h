#pragma once

#include <cstddef>
#include <string>
#include <vector>
class Buffer {
public:
    static constexpr std::size_t kCheapPrepend = 8;
    static constexpr std::size_t kInitialSize = 1024;

    explicit Buffer(std::size_t initial_size = kInitialSize);
    ~Buffer() = default;

    // 禁止拷贝
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    // 允许移动
    Buffer(Buffer&&) noexcept = default;
    Buffer& operator=(Buffer&&) noexcept = default;
    // 状态查询
    [[nodiscard]] std::size_t readable_bytes() const noexcept;
    [[nodiscard]] std::size_t writable_bytes() const noexcept;
    [[nodiscard]] std::size_t prependable_bytes() const noexcept;
    [[nodiscard]] const char* peek() const noexcept;

    // 写入数据
    void append(std::string_view data);
    void append(const char* data, std::size_t len);
    [[nodiscard]] char* begin_write() noexcept;
    [[nodiscard]] const char* begin_write() const noexcept;

    void has_written(std::size_t len);

    // 消费数据
    void retrieve(std::size_t len);
    void retrieve_until(const char* end);
    void retrieve_all();
    [[nodiscard]] std::string retrieve_as_string(std::size_t len);
    [[nodiscard]] std::string retrieve_all_as_string();

    const char* find_crlf() const noexcept;
    const char* find_crlf(const char* start) const noexcept;

private:
    void ensure_writable_bytes(std::size_t len);
    void make_space(std::size_t len);
    char* begin() noexcept {
        return buffer_.data();
    }
    const char* begin() const noexcept {
        return buffer_.data();
    }

private:
    std::vector<char> buffer_;
    std::size_t reader_index_;
    std::size_t writer_index_;
};