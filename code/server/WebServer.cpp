#include "WebServer.h"

WebServer::WebServer() : is_close_(false), timer_(new HeapTimer), epoller_(new Epoller()) {
    src_dir_ = getcwd(nullptr, 256);
    assert(src_dir_);
    std::string config_path = src_dir_;
    strncat(src_dir_, "/resources", 16);
    HttpConn::user_count_ = 0;
    HttpConn::src_dir_ = src_dir_;

    config_path += "/config.json";
    std::ifstream ifstrm(config_path);
    if (!ifstrm.is_open()) {
        LOG_ERROR("Can't open %s", config_path.c_str());
    }
    std::string json_str;
    while (ifstrm) {
        std::string line;
        std::getline(ifstrm, line);
        json_str += line + "\n";
    }

    std::string err_msg;
    Json json = Json::parse(json_str, err_msg);
     if (err_msg != "") {
        
    }

    port_ = json["webserver"]["port"].toNumber();
    open_linger_ = json["webserver"]["opt_linger"].toBool();
    time_out_ms_ = json["webserver"]["time_out_ms"].toNumber();
    int trig_mode = json["webserver"]["trig_mode"].toNumber();
    init_event_mode(trig_mode);

    std::string sql_IP = json["mysql"]["ip"].toString();
    std::string sql_user = json["mysql"]["user"].toString();
    std::string sql_pwd = json["mysql"]["passwd"].toString();
    std::string db_name = json["mysql"]["db_name"].toString();
    unsigned short port = json["mysql"]["port"].toNumber();
    int conn_poll_num =  json["mysql"]["max_conn"].toNumber();
    int sql_time_out = json["mysql"]["time_out"].toNumber();
    ConnectPool::get_connect_pool()->init(sql_IP, sql_user, sql_pwd, db_name, port, conn_poll_num, sql_time_out);

    int thread_num = json["thread"]["thread_num"].toNumber();
    std::unique_ptr<ThreadPool> new_thread_pool(new ThreadPool(thread_num));
    thread_pool_ = std::move(new_thread_pool);

    if (!init_socket()) {
        is_close_ = true;
    }

    bool open_log = json["log"]["open_log"].toBool();
    if (open_log) {
        int log_level = json["log"]["log_level"].toNumber();
        int log_que_size = json["log"]["log_que_size"].toNumber();
        Log::get_log()->init(log_level, "./log", ".log", log_que_size);
        if (is_close_) {
            LOG_ERROR("========== Server init error!==========");
        } else {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Port:%d, OpenLinger: %s", port_, open_linger_ ? "true":"false");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                            (listen_event_ & EPOLLET ? "ET": "LT"),
                            (conn_event_ & EPOLLET ? "ET": "LT"));
            LOG_INFO("LogSys level: %d", log_level);
            LOG_INFO("srcDir: %s", HttpConn::src_dir_);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", conn_poll_num, thread_num);
        }
    }
}

WebServer::WebServer(
        int port, int trig_mode, int time_out_ms, bool opt_linger,
        const char* sql_IP, int sql_port, const char* sql_user, const char* sql_pwd,
        const char* db_name, int conn_poll_num, int sql_time_out, int thread_num,
        bool open_log, int log_level, int log_que_size
        ) :
        port_(port), open_linger_(opt_linger), time_out_ms_(time_out_ms), is_close_(false),
        thread_pool_(new ThreadPool(thread_num)), timer_(new HeapTimer), epoller_(new Epoller()) {
    src_dir_ = getcwd(nullptr, 256);
    assert(src_dir_);
    strncat(src_dir_, "/resources", 16);
    HttpConn::user_count_ = 0;
    HttpConn::src_dir_ = src_dir_;
    ConnectPool::get_connect_pool()->init(sql_IP, sql_user, sql_pwd, db_name, port, conn_poll_num, sql_time_out);
    init_event_mode(trig_mode);
    if (!init_socket()) {
        is_close_ = true;
    }

    if (open_log) {
        Log::get_log()->init(log_level, "./log", ".log", log_que_size);
        if (is_close_) {
            LOG_ERROR("========== Server init error!==========");
        } else {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Port:%d, OpenLinger: %s", port_, opt_linger ? "true":"false");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                            (listen_event_ & EPOLLET ? "ET": "LT"),
                            (conn_event_ & EPOLLET ? "ET": "LT"));
            LOG_INFO("LogSys level: %d", log_level);
            LOG_INFO("srcDir: %s", HttpConn::src_dir_);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", conn_poll_num, thread_num);
        }
    }
}

WebServer::~WebServer() {
    close(listen_fd_);
    is_close_ = true;
    free(src_dir_);
}

void WebServer::init_event_mode(int trig_mode) {
    listen_event_ = EPOLLRDHUP;
    conn_event_ = EPOLLONESHOT | EPOLLRDHUP;
    switch (trig_mode)
    {
    case 0:
        break;
    case 1:
        conn_event_ |= EPOLLET;
        break;
    case 2:
        listen_event_ |= EPOLLET;
        break;
    case 3:
        listen_event_ |= EPOLLET;
        conn_event_ |= EPOLLET;
        break;
    default:
        listen_event_ |= EPOLLET;
        conn_event_ |= EPOLLET;
        break;
    }
    HttpConn::is_ET_ = (conn_event_ & EPOLLET);
}

bool WebServer::init_socket() {
    sockaddr_in addr;
    if (port_ > 65535 || port_ < 1024) {
        LOG_ERROR("Port:%d error!",  port_);
        return false;
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);
    struct linger opt_linger = { 0 };
    if (open_linger_) {
        opt_linger.l_onoff = 1;
        opt_linger.l_linger = 1;
    }
    listen_fd_ = socket(PF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0) {
        LOG_ERROR("Create socket error!", port_);
        return false;
    }

    int ret = setsockopt(listen_fd_, SOL_SOCKET, SO_LINGER, &opt_linger, sizeof(opt_linger));
    if (ret < 0) {
        close(listen_fd_);
        LOG_ERROR("Init linger error!", port_);
        return false;
    }

    int optval = 1;
    ret = setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
    if (ret == -1) {
        close(listen_fd_);
        LOG_ERROR("set socket setsockopt error !");
        return false;
    }

    ret = bind(listen_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if (ret < 0) {
        close(listen_fd_);
        LOG_ERROR("Bind Port:%d error!", port_);
        return false;
    }

    ret = listen(listen_fd_, 6);
    if (ret < 0) {
        close(listen_fd_);
        LOG_ERROR("Listen port:%d error!", port_);
        return false;
    }

    ret = epoller_->add_fd(listen_fd_, listen_event_ | EPOLLIN);
    if (ret == 0) {
        close(listen_fd_);
        LOG_ERROR("Add listen error!");
        return false;
    }
    set_fd_nonblock(listen_fd_);
    LOG_INFO("Server port:%d", port_);
    return true;
}

int WebServer::set_fd_nonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}

void WebServer::start() {
    int time_ms = -1;
    if(!is_close_) {
        LOG_INFO("========== Server start ==========");
    }
    while (!is_close_) {
        if (time_out_ms_ > 0) {
            time_ms = timer_->get_next_tick();
        }
        int event_cnt = epoller_->wait(time_ms);
        for (int i = 0; i != event_cnt; ++i) {
            int fd = epoller_->get_event_fd(i);
            std::uint32_t events = epoller_->get_events(i);
            if (fd == listen_fd_) {
                deal_listen();
            } else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                assert(users_.count(fd) > 0);
                close_conn(&users_[fd]);
            } else if (events & EPOLLIN) {
                assert(users_.count(fd) > 0);
                deal_read_(&users_[fd]);
            } else if (events & EPOLLOUT) {
                assert(users_.count(fd) > 0);
                deal_write_(&users_[fd]);
            } else {
                LOG_ERROR("Unexpected event");
            }
        }
    }
}

void WebServer::deal_listen() {
    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do {
        int fd = accept(listen_fd_, reinterpret_cast<sockaddr*>(&addr), &len);
        if (fd <= 0) {
            return;
        } else if (HttpConn::user_count_ >= MAX_FD_) {
            send_error(fd, "Server busy!");
            LOG_WARN("Clients is full!");
            return;
        }
        add_client(fd, addr);
    } while (listen_event_ & EPOLLET);
}

void WebServer::send_error(int fd, const char* info) {
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if (ret < 0) {
        LOG_WARN("send error to client[%d] error!", fd);
    }
    close(fd);
}

void WebServer::add_client(int fd, sockaddr_in addr) {
    assert(fd > 0);
    users_[fd].init(fd, addr);
    if (time_out_ms_ > 0) {
        timer_->add(fd, time_out_ms_, std::bind(&WebServer::close_conn, this, &users_[fd]));
    }
    epoller_->add_fd(fd, EPOLLIN | conn_event_);
    set_fd_nonblock(fd);
    LOG_INFO("Client[%d] in!", users_[fd].get_fd());
}

void WebServer::close_conn(HttpConn* client) {
    assert(client);
    LOG_INFO("Client[%d] quit!", client->get_fd());
    epoller_->del_fd(client->get_fd());
    client->close_conn();
}

void WebServer::deal_read_(HttpConn* client) {
    assert(client);
    extent_time_(client);
    thread_pool_->commit(std::bind(&WebServer::on_read, this, client));
}

void WebServer::deal_write_(HttpConn* client) {
    assert(client);
    extent_time_(client);
    thread_pool_->commit(std::bind(&WebServer::on_write, this, client));
}

void WebServer::on_read(HttpConn* client) {
    assert(client);
    int ret = -1;
    int read_errno = 0;
    ret = client->read(&read_errno);
    if (ret <= 0 && read_errno != EAGAIN) {
        close_conn(client);
        return;
    }
    on_process(client);
}

void WebServer::on_write(HttpConn* client) {
    assert(client);
    int ret = -1;
    int write_errno = 0;
    ret = client->write(&write_errno);
    if (client->to_write_bytes() == 0) {
        if (client->is_keep_alive()) {
            on_process(client);
            return;
        }
    } else if (ret < 0) {
        if (write_errno == EAGAIN) {
            epoller_->mod_fd(client->get_fd(), conn_event_ | EPOLLOUT);
            return;
        }
    }
    close_conn(client);
}

void WebServer::on_process(HttpConn* client) {
    if (client->process()) {
        epoller_->mod_fd(client->get_fd(), conn_event_ | EPOLLOUT);
    } else {
        epoller_->mod_fd(client->get_fd(), conn_event_ | EPOLLIN);
    }
}

void WebServer::extent_time_(HttpConn* client) {
    assert(client);
    if(time_out_ms_ > 0) {
        timer_->adjust(client->get_fd(), time_out_ms_); }
}