#include "ConnectPool.h"

ConnectPool* ConnectPool::get_connect_pool() {
    static ConnectPool pool;
    return &pool;
}

void ConnectPool::init(std::string ip, std::string user,
                         std::string passwd, std::string db_name,
                         unsigned short port, int max_conn,
                         int time_out) {
    ip_ = ip;
    user_ = user;
    passwd_ = passwd;
    db_name_ = db_name;
    port_ = port;
    max_conn_ = max_conn;
    time_out = time_out;
    for (int i = 0; i != max_conn_; ++i) {
        add_connection();
    }
}

void ConnectPool::add_connection() {
    MySQLConn* conn = new MySQLConn;
    conn->connect(user_, passwd_, db_name_, ip_, port_);
    mysql_conn_que_.push(conn);
}

ConnectPool::ConnectPool() { }

ConnectPool::~ConnectPool() {
    std::lock_guard<std::mutex> locker(mtx_);
    while (!mysql_conn_que_.empty()) {
        MySQLConn* conn = mysql_conn_que_.front();
        mysql_conn_que_.pop();
        delete conn;
    }
    mysql_library_end();
}

std::shared_ptr<MySQLConn> ConnectPool::get_connection() {
    std::unique_lock<std::mutex> locker(mtx_);
     while (mysql_conn_que_.empty()) {
        if (std::cv_status::timeout == cv_.wait_for(locker, std::chrono::milliseconds(time_out_))) {
            if (mysql_conn_que_.empty()) {
                return nullptr;
                // continue;
            }
        }
     }
    std::shared_ptr<MySQLConn> conn_ptr(mysql_conn_que_.front(), [this](MySQLConn* conn){
        std::lock_guard<std::mutex> locker(mtx_);
        mysql_conn_que_.push(conn);
        cv_.notify_all();
    });
    mysql_conn_que_.pop();
    return conn_ptr;
}