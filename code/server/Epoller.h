#pragma once

#include <sys/epoll.h>
#include <unistd.h>
#include <vector>
#include <cassert>

class Epoller {
public:
    explicit Epoller(int max_event = 1024); // 构造函数，max_event 为 events_ 数组长度，调用 epoll_create 初始化 epoll 例程的文件描述符 epoll_fd_
    ~Epoller(); // 析构函数，调用 close 关闭 epoll 例程的文件描述符 epoll_fd_

    bool add_fd(int fd, uint32_t events); // 将监测事件为 events，文件描述符为 fd 的 epoll_event 注册到 epoll 例程
    bool mod_fd(int fd, uint32_t events); // 修改 epoll 例程中文件描述符为 fd 的 epoll_event 的事件为 events
    bool del_fd(int fd); // 从 epoll 例程中删除文件描述符为 fd 的 epoll_event

    int wait(int time_out_ms = -1); // 等待 epoll_event 事件的发生，time_out_ms = -1 表示一直阻塞直到事件发生
    int get_event_fd(std::size_t i) const; // 获取 events_ 数组索引为 i 的 epoll_event 的文件描述符
    uint32_t get_events(std::size_t i) const; // 获取 events_ 数组索引为 i 的 epoll_event 的事件

private:
    int epoll_fd_; // epoll 例程的文件描述符
    std::vector<epoll_event> events_; // 发生变化的文件描述符的 epoll_event 将被填入该数组
};