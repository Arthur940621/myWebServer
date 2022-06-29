#pragma once

#include <unordered_map>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "../buffer/Buffer.h"
#include "../log/Log.h"

class HttpResponse {
public:
    HttpResponse();
    ~HttpResponse();

    void unmap_file();
    void init(const std::string& src_dir, std::string& path, bool is_keep_alive = false, int code = -1);
    void make_response(Buffer& buffer);
    void error_content(Buffer& buffer, const std::string& message);

    char* file();
    std::size_t file_len() const;

private:
    void add_state_line(Buffer& buffer);
    void add_header(Buffer& buffer);
    void add_content(Buffer& buffer);

    std::string get_file_type();

    int code_;
    bool is_keep_alive_;
    std::string path_;
    std::string src_dir_;
    char* mm_file_;
    struct stat mm_file_stat_;

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int, std::string> CODE_PATH;
};