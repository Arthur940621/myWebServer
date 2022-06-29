#include "HttpConn.h"

const char* HttpConn::src_dir_;
std::atomic<int> HttpConn::user_count_;
bool HttpConn::is_ET_;

HttpConn::HttpConn() : fd_(-1), addr_({0}), is_close_(true) {

}

void HttpConn::init(int sock_fd, const sockaddr_in& addr) {
    assert(sock_fd > 0);
    user_count_++;
    addr_ = addr;
    fd_ = sock_fd;
    read_buff_.retrieve_all();
    write_buff_.retrieve_all();
    is_close_ = false;
    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", fd_, get_IP(), get_port(), static_cast<int>(user_count_));
}

HttpConn::~HttpConn() {
    close_conn();
}

void HttpConn::close_conn() {
    response_.unmap_file();
    if (is_close_ == false) {
        is_close_ = true;
        user_count_--;
        close(fd_);
        LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", fd_, get_IP(), get_port(), static_cast<int>(user_count_));
    }
}

ssize_t HttpConn::read(int* save_errno) {
    ssize_t len = -1;
    do {
        len = read_buff_.read_fd(fd_, save_errno);
        if (len < 0) {
            break;
        }
    } while (is_ET_);
    return len;
}

ssize_t HttpConn::write(int* save_errno) {
    ssize_t len = -1;
    do {
        len = writev(fd_, iov_, iov_cnt_);
        if (len <= 0) {
            *save_errno = errno;
            break;
        }
        if (iov_[0].iov_len + iov_[1].iov_len == 0) {
            break;
        } else if (static_cast<std::size_t>(len) > iov_[0].iov_len) {
            iov_[1].iov_base = reinterpret_cast<uint8_t*>(iov_[1].iov_base) + (len - iov_[0].iov_len);
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            if (iov_[0].iov_len) {
                write_buff_.retrieve_all();
                iov_[0].iov_len = 0;
            }
        } else {
            iov_[0].iov_base = reinterpret_cast<uint8_t*>(iov_[0].iov_base) + len;
            iov_[0].iov_len -= len;
            write_buff_.retrieve(len);
        }
    } while (is_ET_ || to_write_bytes() > 10240);
    return len;
}

int HttpConn::get_fd() const {
    return fd_;
}

int HttpConn::get_port() const {
    return addr_.sin_port;
}

const char* HttpConn::get_IP() const {
    return inet_ntoa(addr_.sin_addr);
}

sockaddr_in HttpConn::get_addr() const {
    return addr_;
}

bool HttpConn::process() {
    request_.init();
    if (read_buff_.readable_bytes() <= 0) {
        return false;
    } else if (request_.parse(read_buff_)) {
        LOG_DEBUG("%s", request_.path().c_str());
        response_.init(src_dir_, request_.path(), request_.is_keep_alive(), 200);
    } else {
        response_.init(src_dir_, request_.path(), false, 400);
    }

    response_.make_response(write_buff_);
    iov_[0].iov_base = const_cast<char*>(write_buff_.peek());
    iov_[0].iov_len = write_buff_.readable_bytes();
    iov_cnt_ = 1;

    if (response_.file_len() > 0 && response_.file()) {
        iov_[1].iov_base = response_.file();
        iov_[1].iov_len = response_.file_len();
        iov_cnt_ = 2;
    }
    LOG_DEBUG("filesize:%d, %d  to %d", response_.file_len() , iov_cnt_, to_write_bytes());
    return true;
}

int HttpConn::to_write_bytes() {
    return iov_[0].iov_len + iov_[1].iov_len;
}

bool HttpConn::is_keep_alive() const {
    return request_.is_keep_alive();
}