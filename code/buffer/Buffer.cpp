#include "Buffer.h"

Buffer::Buffer(int buf_size) : buffer_(buf_size, 0), read_pos_(0), write_pos_(0) { }

std::size_t Buffer::readable_bytes() const {
    return write_pos_ - read_pos_;
}

std::size_t Buffer::writable_bytes() const {
    return buffer_.size() - write_pos_;
}

std::size_t Buffer::prependable_bytes() const {
    return read_pos_;
}

const char* Buffer::peek() const {
    return begin_ptr() + read_pos_;
}

char* Buffer::begin_ptr() {
    return &*buffer_.begin();
}

const char* Buffer::begin_ptr() const {
    return &*buffer_.begin();
}

void Buffer::retrieve(std::size_t len) {
    assert(len <= readable_bytes());
    read_pos_ += len;
}

void Buffer::retrieve_until(const char* end) {
    assert(peek() <= end);
    retrieve(end - peek());
}

void Buffer::retrieve_all() {
    bzero(&buffer_[0], buffer_.size());
    read_pos_ = 0;
    write_pos_ = 0;
}

std::string Buffer::retrieve_all_to_str() {
    std::string str(peek(), readable_bytes());
    retrieve_all();
    return str;
}

const char* Buffer::begin_write() const {
    return begin_ptr() + write_pos_;
}

char* Buffer::begin_write() {
    return begin_ptr() + write_pos_;
}

void Buffer::make_space(std::size_t len) {
    if (writable_bytes() + prependable_bytes() < len) {
        buffer_.resize(write_pos_ + len + 1);
    } else {
        std::size_t readable = readable_bytes();
        std::copy(begin_ptr() + read_pos_, begin_ptr() + write_pos_, begin_ptr());
        read_pos_ = 0;
        write_pos_ = read_pos_ + readable;
        assert(readable == readable_bytes());
    }
}

void Buffer::ensure_writeable(std::size_t len) {
    if (writable_bytes() < len) {
        make_space(len);
    }
    assert(writable_bytes() >= len);
}

void Buffer::has_written(std::size_t len) {
    write_pos_ += len;
}

void Buffer::append(const char* str, std::size_t len) {
    assert(str);
    ensure_writeable(len);
    std::copy(str, str + len, begin_write());
    has_written(len);
}

void Buffer::append(const std::string& str) {
    append(str.data(), str.length());
}

void Buffer::append(const void* data, std::size_t len) {
    assert(data);
    append(static_cast<const char*>(data), len);
}

void Buffer::append(const Buffer& buff) {
    append(buff.peek(), buff.readable_bytes());
}

ssize_t Buffer::read_fd(int fd, int* save_errno) {
    char extra_buff[65535];
    iovec iov[2];
    std::size_t writable = writable_bytes();
    iov[0].iov_base = begin_write();
    iov[0].iov_len = writable;
    iov[1].iov_base = extra_buff;
    iov[1].iov_len = sizeof(extra_buff);

    int iov_cnt = (writable < sizeof(extra_buff)) ? 2 : 1;
    ssize_t len = readv(fd, iov, iov_cnt);
    if (len < 0) {
        *save_errno = errno;
    } else if (static_cast<std::size_t>(len) <= writable) {
        write_pos_ += len;
    } else {
        write_pos_ = buffer_.size();
        append(extra_buff, len - writable);
    }
    return len;
}

ssize_t Buffer::write_fd(int fd, int* save_errno) {
    std::size_t readable = readable_bytes();
    ssize_t len = write(fd, peek(), readable);
    if (len < 0) {
        *save_errno = errno;
    } else {
        read_pos_ += len;
    }
    return len;
}