#include "net/Buffer.h"

#include <algorithm>
#include <assert.h>

Buffer::Buffer(std::size_t initial_size)
    : buffer_(kCheapPrepend + initial_size), reader_index_(kCheapPrepend),
      writer_index_(kCheapPrepend) {}

std::size_t Buffer::readable_bytes() const noexcept {
    return writer_index_ - reader_index_;
}
std::size_t Buffer::writable_bytes() const noexcept {
    return buffer_.size() - writer_index_;
}

std::size_t Buffer::prependable_bytes() const noexcept {
    return reader_index_;
}
const char* Buffer::peek() const noexcept {
    return buffer_.data() + reader_index_;
}
void Buffer::append(std::string_view data) {
    append(data.data(), data.length());
}

void Buffer::append(const char* data, std::size_t len) {
    ensure_writable_bytes(len);
    std::copy(data, data + len, begin_write());
    has_written(len);
}

char* Buffer::begin_write() noexcept {
    return buffer_.data() + writer_index_;
}
const char* Buffer::begin_write() const noexcept {
    return buffer_.data() + writer_index_;
}
void Buffer::has_written(std::size_t len) {
    assert(len <= writable_bytes());
    writer_index_ += len;
}
void Buffer::retrieve(std::size_t len) {
    assert(len <= readable_bytes());
    reader_index_ += len;
}
void Buffer::retrieve_until(const char* end) {
    assert(peek() <= end);
    assert(end <= begin_write());
    retrieve(static_cast<std::size_t>(end - peek()));
}

void Buffer::retrieve_all() {
    reader_index_ = kCheapPrepend;
    writer_index_ = kCheapPrepend;
}

std::string Buffer::retrieve_as_string(std::size_t len) {
    assert(len <= readable_bytes());
    std::string ret(peek(), len);
    retrieve(len);
    return ret;
}
std::string Buffer::retrieve_all_as_string() {
    auto ret = retrieve_as_string(readable_bytes());
    reader_index_ = kCheapPrepend;
    writer_index_ = kCheapPrepend;
    return ret;
}

void Buffer::ensure_writable_bytes(std::size_t len) {
    if (writable_bytes() < len) {
        make_space(len);
    }
}

void Buffer::make_space(std::size_t len) {
    if (writable_bytes() + prependable_bytes() < len + kCheapPrepend) {
        buffer_.resize(writer_index_ + len);
    } else {
        const auto readable = readable_bytes();
        std::copy(begin() + reader_index_, begin_write(), begin() + kCheapPrepend);
        reader_index_ = kCheapPrepend;
        writer_index_ = reader_index_ + readable;
    }
    assert(writable_bytes() >= len);
}

const char* Buffer::find_crlf() const noexcept {
    return find_crlf(peek());
}
const char* Buffer::find_crlf(const char* start) const noexcept {
    assert(peek() <= start);
    assert(start <= begin_write());

    const char* search = start;
    while (search < begin_write()) {
        const char* cr = std::find(search, begin_write(), '\r');
        if (cr == begin_write() || cr + 1 == begin_write()) {
            return nullptr;
        }

        if (*(cr + 1) == '\n') {
            return cr;
        }

        search = cr + 1;
    }

    return nullptr;
}
