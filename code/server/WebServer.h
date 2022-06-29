#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>
#include <fstream>
#include "../pool/ConnectPool.h"
#include "../pool/ThreadPool.h"
#include "../http/HttpConn.h"
#include "../json/Json.h"
#include "Epoller.h"
#include "../log/Log.h"
#include "../timer/Timer.h"

class WebServer {
public:
    WebServer();
    WebServer(
        int port, int trig_mode, int time_out_ms, bool opt_linger,
        const char* sql_IP, int sql_port, const char* sql_user, const char* sql_pwd,
        const char* db_name, int conn_poll_num, int sql_time_out_, int thread_num,
        bool open_log, int log_level, int log_que_size
        );
    ~WebServer();
    void start();

private:
    bool init_socket();
    void init_event_mode(int trig_mode);
    void deal_listen();
    void deal_read_(HttpConn* client);
    void deal_write_(HttpConn* client);
    void send_error(int fd, const char*info);
    void add_client(int fd, sockaddr_in addr);
    void close_conn(HttpConn* client);

    void on_read(HttpConn* client);
    void on_write(HttpConn* client);
    void on_process(HttpConn* client);
    void extent_time_(HttpConn* client);

    static int set_fd_nonblock(int fd);
    static const int MAX_FD_ = 65536;

    int port_;
    bool open_linger_;
    int time_out_ms_;
    bool is_close_;
    int listen_fd_;
    char* src_dir_;

    std::uint32_t listen_event_;
    std::uint32_t conn_event_;

    std::unique_ptr<ThreadPool> thread_pool_;
    std::unique_ptr<HeapTimer> timer_;
    std::unique_ptr<Epoller> epoller_;
    std::unordered_map<int, HttpConn> users_;
};