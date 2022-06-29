#pragma once

#include <sys/uio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include "HttpRequest.h"
#include "HttpResponse.h"

class HttpConn {
public:
    HttpConn();
    ~HttpConn();

    void init(int sock_fd, const sockaddr_in& addr);
    ssize_t read(int* save_errno);
    ssize_t write(int* save_errno);
    void close_conn();
    int get_fd() const;
    int get_port() const;
    const char* get_IP() const;
    sockaddr_in get_addr() const;
    bool process();
    int to_write_bytes();
    bool is_keep_alive() const;

    static bool is_ET_;
    static const char* src_dir_;
    static std::atomic<int> user_count_;

private:
    int fd_;
    struct  sockaddr_in addr_;
    bool is_close_;
    int iov_cnt_;
    struct iovec iov_[2];
    Buffer read_buff_;
    Buffer write_buff_;
    HttpRequest request_;
    HttpResponse response_;
};