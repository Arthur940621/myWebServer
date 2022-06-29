#pragma once

#include <unordered_map>
#include <unordered_set>
#include <regex>
#include "../pool/ConnectPool.h"
#include "../buffer/Buffer.h"
#include "../coder/Coder.h"
#include "../log/Log.h"

class HttpRequest{
public:
    // 状态机，标识解析位置
    enum class PARSE_STATE {
        REQUEST_LINE, // 解析请求行
        HEADERS, // 解析请求头
        BODY, // 解析消息体，仅用于解析 POST 请求
        FINISH, // 结束
    };

    HttpRequest();
    ~HttpRequest() = default;

    void init();
    bool parse(Buffer& buffer);

    std::string path() const;
    std::string& path();
    std::string method() const;
    std::string version() const;
    std::string get_post(const std::string& key) const;
    std::string get_post(const char* key) const;

    bool is_keep_alive() const;

private:
    bool parse_request_line(const std::string& line);
    void parse_header(const std::string& line);
    void parse_body(const std::string& line);

    void parse_path();
    void parse_post();
    void parse_from_urlencoded();

    static bool user_verify(const std::string& name, const std::string& pwd, bool is_login);

    PARSE_STATE state_;
    std::string method_, path_, version_, body_;
    std::unordered_map<std::string, std::string> header_;
    std::unordered_map<std::string, std::string> post_;
    static Coder coder_;
    static ConnectPool* connect_pool;

    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
};