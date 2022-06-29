#pragma once

#include "Json.h"
#include "JsonException.h"

constexpr bool is1to9(char ch) { return ch >= '1' && ch <= '9'; }
constexpr bool is0to9(char ch) { return ch >= '0' && ch <= '9'; }

class JsonParser {
public:
    explicit JsonParser(const char* cstr) : _start(cstr), _curr(cstr) { }
    explicit JsonParser(const std::string& content) : _start(content.c_str()), _curr(content.c_str()) { }

    JsonParser(const JsonParser&) = delete;
    JsonParser& operator=(const JsonParser&) = delete;

    Json parse();

private:
    Json parseValue();
    Json parseLiteral(const std::string& literal);
    Json parseNumber();
    Json parseString();
    Json parseArray();
    Json parseObject();

    void parseWhitespace();
    unsigned parse4hex();
    std::string encodeUTF8(unsigned u);
    std::string parseRawString();

    const char* _start;
    const char* _curr;
};
