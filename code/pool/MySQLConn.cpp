#include "MySQLConn.h"

MySQLConn::MySQLConn() : mysql_conn_(mysql_init(nullptr)) {
    mysql_set_character_set(mysql_conn_, "utf8mb4");
}

MySQLConn::~MySQLConn() {
    if (mysql_conn_ != nullptr) {
        mysql_close(mysql_conn_);
    }
    freeResult();
}

bool MySQLConn::connect(const std::string& user, const std::string& password, const std::string& db_name, const std::string& ip, unsigned short port) {
    MYSQL* ptr = mysql_real_connect(mysql_conn_, ip.c_str(), user.c_str(), password.c_str(), db_name.c_str(), port, nullptr, 0);
    return ptr != nullptr;
}

bool MySQLConn::update(const std::string& sql) {
    if (mysql_query(mysql_conn_, sql.c_str())) {
        return false;
    }
    return true;
}

bool MySQLConn::query(const std::string& sql) {
    freeResult();
    if (mysql_query(mysql_conn_, sql.c_str())) {
        return false;
    }
    mysql_res_ = mysql_store_result(mysql_conn_);
    return true;
}

bool MySQLConn::next() {
    if (mysql_res_ != nullptr) {
        mysql_row_ = mysql_fetch_row(mysql_res_);
        if (mysql_row_ != nullptr) {
            return true;
        }
    }
    return false;
}

std::string MySQLConn::value(unsigned int index) {
    unsigned int row_count = mysql_num_fields(mysql_res_);
    assert(index >= 0 && index < row_count);
    char* val = mysql_row_[index];
    unsigned long length =  mysql_fetch_lengths(mysql_res_)[index];
    return std::string(val, length);
}

bool MySQLConn::transaction() {
    return mysql_autocommit(mysql_conn_, false);
}

bool MySQLConn::commit() {
    return mysql_commit(mysql_conn_);
}

bool MySQLConn::roollback() {
    return mysql_rollback(mysql_conn_);
}

void MySQLConn::freeResult() {
    if (mysql_res_) {
        mysql_free_result(mysql_res_);
        mysql_res_ = nullptr;
    }
}