#pragma once

#include <mysql/mysql.h>
#include <string>
#include <cassert>
#include <chrono>

class MySQLConn {
public:
    MySQLConn(); // 初始化数据库链接
    ~MySQLConn(); // 释放数据库链接

    bool connect(const std::string& user, const std::string& password, const std::string& db_name, const std::string& ip, unsigned short port); // 链接数据库
    bool update(const std::string& sql); // 更新数据库
    bool query(const std::string& sql); // 查询数据库
    bool next(); // 遍历查询得到的结果集
    std::string value(unsigned int index); // 得到结果集中的字段值
    bool transaction(); // 事务操作
    bool commit(); // 事务提交
    bool roollback(); // 事务回滚

private:
    void freeResult();
    MYSQL* mysql_conn_ = nullptr;
    MYSQL_RES* mysql_res_ = nullptr;
    MYSQL_ROW mysql_row_ = nullptr;
};