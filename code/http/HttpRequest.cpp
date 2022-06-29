#include "HttpRequest.h"

Coder HttpRequest::coder_ = Coder();
ConnectPool* HttpRequest::connect_pool = ConnectPool::get_connect_pool();

const std::unordered_set<std::string> HttpRequest::DEFAULT_HTML {
            "/index", "/register", "/login",
             "/welcome", "/video", "/picture"
        };

const std::unordered_map<std::string, int> HttpRequest::DEFAULT_HTML_TAG {
            {"/register.html", 0}, {"/login.html", 1}
        };

HttpRequest::HttpRequest() {
    init();
}

void HttpRequest::init() {
    method_ = path_ = version_ = body_ = "";
    state_ = PARSE_STATE::REQUEST_LINE;
    header_.clear();
    post_.clear();
}

std::string HttpRequest::path() const {
    return path_;
}

std::string& HttpRequest::path() {
    return path_;
}

std::string HttpRequest::method() const {
    return method_;
}

std::string HttpRequest::version() const {
    return version_;
}

std::string HttpRequest::get_post(const std::string& key) const {
    assert(key != "");
    if(post_.count(key) == 1) {
        return post_.find(key)->second;
    }
    return "";
}

std::string HttpRequest::get_post(const char* key) const {
    assert(key != nullptr);
    if(post_.count(key) == 1) {
        return post_.find(key)->second;
    }
    return "";
}

bool HttpRequest::parse(Buffer& buffer) {
    const char CRLF[] = "\r\n";
    if (buffer.readable_bytes() <= 0) {
        return false;
    }
    while (buffer.readable_bytes() && state_ != PARSE_STATE::FINISH) {
        const char* line_end = std::search(buffer.peek(), static_cast<const char*>(buffer.begin_write()), CRLF, CRLF+2);
        std::string line(buffer.peek(), line_end);
        switch (state_)
        {
        case PARSE_STATE::REQUEST_LINE:
            if (!parse_request_line(line)) {
                return false;
            }
            parse_path();
            break;
        case PARSE_STATE::HEADERS:
            parse_header(line);
            if (buffer.readable_bytes() <= 2) {
                state_ = PARSE_STATE::FINISH;
            }
            break;
        case PARSE_STATE::BODY:
            parse_body(line);
            break;
        default:
            break;
        }
        if (line_end == buffer.begin_write()) {
            buffer.retrieve_until(line_end);
            break;
        }
        buffer.retrieve_until(line_end + 2);
    }
    LOG_DEBUG("[%s], [%s], [%s]", method_.c_str(), path_.c_str(), version_.c_str());
    return true;
}

bool HttpRequest::parse_request_line(const std::string& line) {
    std::regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch sub_match;
    if (std::regex_match(line, sub_match, patten)) {
        method_ = sub_match[1];
        path_ = sub_match[2];
        version_ = sub_match[3];
        state_ = PARSE_STATE::HEADERS;
        return true;
    }
    LOG_ERROR("RequestLine Error");
    return false;
}

void HttpRequest::parse_path() {
    if (path_ == "/") {
        path_ = "/index.html";
    } else {
        for (auto& item : DEFAULT_HTML) {
            if (item == path_) {
                path_ += ".html";
                break;
            }
        }
    }
}

void HttpRequest::parse_header(const std::string& line) {
    std::regex patten("^([^:]*): ?(.*)$");
    std::smatch sub_match;
    if (std::regex_match(line, sub_match, patten)) {
        header_[sub_match[1]] = sub_match[2];
    } else {
        state_ = PARSE_STATE::BODY;
    }
}

void HttpRequest::parse_body(const std::string& line) {
    body_ = line;
    parse_post();
    state_ = PARSE_STATE::FINISH;
    LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
}

void HttpRequest::parse_post() {
    if (method_ == "POST" && header_["Content-Type"] == "application/x-www-form-urlencoded") {
        parse_from_urlencoded();
        if (DEFAULT_HTML_TAG.count(path_)) {
            int tag = DEFAULT_HTML_TAG.find(path_)->second;
            LOG_DEBUG("Tag:%d", tag);
            if (tag == 0 || tag == 1) {
                bool is_login = (tag == 1);
                if (user_verify(post_["username"], post_["password"], is_login)) {
                    path_ = "/welcome.html";
                } else {
                    path_ = "/error.html";
                }
            }
        }
    }
}

void HttpRequest::parse_from_urlencoded() {
    if (body_.size() == 0) {
        return;
    }

    std::string key, value;
    int n = body_.size(), i = 0, j = 0;
    for (; i < n; ++i) {
        char ch = body_[i];
        switch (ch)
        {
        case '=':
            key = coder_.url_decode(body_.substr(j, i - j));
            j = i + 1;
            break;
        case '+':
            body_[i] = ' ';
            break;
        case '&':
            value = coder_.url_decode(body_.substr(j, i - j));
            j = i + 1;
            post_[key] = value;
            LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
            break;
        default:
            break;
        }
    }
    assert(j <= i);
    if (post_.count(key) == 0 && j < i) {
        key = coder_.url_decode(key);
        value = coder_.url_decode(body_.substr(j, i - j));
        post_[key] = value;
    }
}

bool HttpRequest::user_verify(const std::string& name, const std::string& pwd, bool is_login) {
    if (name == "" || pwd == "") {
        return false;
    }
    LOG_INFO("Verify name:%s pwd:%s", name.c_str(), pwd.c_str());
    std::shared_ptr<MySQLConn> conn = connect_pool->get_connection();
    assert(conn);

    bool flag = false;
    char order[256] = { 0 };
    snprintf(order, 256, "SELECT username, password FROM user WHERE username='%s' LIMIT 1;", name.c_str());
    LOG_DEBUG("%s", order);

    if (!is_login) {
        flag = true;
    }
    if (!conn->query(order)) {
        return false;
    }
    while (conn->next()) {
        LOG_DEBUG("MYSQL ROW: %s %s", conn->value(0), conn->value(1));
        std::string password(conn->value(1));
        if (is_login) {
            if (pwd == password) {
                flag = true;
            } else {
                flag = false;
                LOG_DEBUG("pwd error!");
            }
        } else {
            flag = false;
            LOG_DEBUG("user used!");
        }
    }

    if (!is_login && flag) {
        LOG_DEBUG("regirster!");
        bzero(order, 256);
        snprintf(order, 256, "INSERT INTO user(username, password) VALUES('%s','%s');", name.c_str(), pwd.c_str());
        LOG_DEBUG( "%s", order);
        if (!conn->update(order)) {
            LOG_DEBUG( "Insert error!");
            flag = false;
        } else {
            flag = true;
        }
    }
    LOG_DEBUG( "UserVerify success!!");
    return flag;
}

bool HttpRequest::is_keep_alive() const {
    if (header_.count("Connection") == 1) {
        return header_.find("Connection")->second == "keep-alive" && version_ == "1.1";
    }
    return false;
}