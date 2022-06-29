#pragma once

#include <mutex>
#include <string>
#include <thread>
#include <sys/stat.h>
#include <cassert>
#include <sys/time.h>
#include <stdarg.h>
#include "../buffer/Buffer.h"
#include "BolckQueue.h"

class Log {
public:
    void init(int level, const char* path = "./log", const char* suffix = ".log", int max_queue_capacity = 1024);

    static Log* get_log();
    static void flush_log_thread();

    void write(int level, const char *format,...);
    void flush();

    int get_level();
    void set_level(int level);
    bool is_open();

private:
    Log();
    void append_log_level_title(int level);
    virtual ~Log();
    void async_write();

private:
    static const int LOG_PATH_LEN = 256;
    static const int LOG_NAME_LEN = 256;
    static const int MAX_LINES = 50000;

    const char* path_;
    const char* suffix_;
    int max_line_;
    int line_count_;
    int to_day_;
    bool is_open_;
    Buffer buff_;
    int level_;
    bool is_async_;
    FILE* fp_;
    
    std::unique_ptr<BlockQueue<std::string>> deque_;
    std::unique_ptr<std::thread> write_thread_;
    std::mutex mtx_;
};

#define LOG_BASE(level, format, ...) \
    do {\
        Log* log = Log::get_log();\
        if (log->is_open() && log->get_level() <= level) {\
            log->write(level, format, ##__VA_ARGS__); \
            log->flush();\
        }\
    } while(0);

#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);