#include "JsonParser.h"
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <cassert>

void JsonParser::parseWhitespace() {
    while (*_curr == ' ' || *_curr == '\t' || *_curr == '\n' || *_curr == '\r') {
        ++_curr;
    }
    _start = _curr;
}

Json JsonParser::parseValue() {
    switch (*_curr) {
    case 'n':
        return parseLiteral("null");
    case 't':
        return parseLiteral("true");
    case 'f':
        return parseLiteral("false");
    case '\"':
        return parseString();
    case '[':
            return parseArray();
    case '{':
        return parseObject();
    case '\0':
        throw JsonException("parse expect value");
    default:
        return parseNumber();
    }
}

Json JsonParser::parseLiteral(const std::string& literal) {
    if (strncmp(_curr, literal.c_str(), literal.size()) != 0) {
        throw JsonException("parse invalid value");
    }
    _curr += literal.size();
    _start = _curr;
    switch (literal[0]) {
    case 't':
        return Json(true);
    case 'f':
        return Json(false);
    default:
        return Json(nullptr);
    }
}

Json JsonParser::parseNumber() {
    if (*_curr == '-') {
        ++_curr;
    }
    if (*_curr == '0') {
        ++_curr;
    } else {
        if (!is1to9(*_curr)) {
            throw JsonException("parse invalid value");
        }
        while (is0to9(*++_curr)) {
            ;
        }
    }
    if (*_curr == '.') {
        if (!is0to9(*++_curr)) {
            throw JsonException("parse invalid value");
        }
        while (is0to9(*++_curr)) {
            ;
        }
    }
    if (*_curr == 'E' || *_curr == 'e') {
        ++_curr;
        if (*_curr == '-' || *_curr == '+') {
            ++_curr;
        }
        if (!is0to9(*_curr)) {
            throw JsonException("parse invalid value");
        }
        while (is0to9(*++_curr)) {
            ;
        }
    }
    double val = strtod(_start, nullptr);
    if (fabs(val) == HUGE_VAL) {
        throw JsonException("parse number too big");
    }
    _start = _curr;
    return Json(val);
}

unsigned JsonParser::parse4hex() {
    unsigned u = 0;
    for (int i = 0; i != 4; ++i) {
        char ch = *++_curr;
        u <<= 4;
        if (ch >= '0' && ch <= '9') {
            u |= ch - '0';
        } else if (ch >= 'A' && ch <= 'F') {
            u |= ch - ('A' - 10);
        } else if (ch >= 'a' && ch <= 'f') {
            u |= ch - ('a' - 10);
        } else {
            throw JsonException("parse invalid unicode hex");
        }
    }
    return u;
}

std::string JsonParser::encodeUTF8(unsigned u) {
    std::string utf8;
    if (u <= 0x7F) {
        utf8.push_back(static_cast<char>(u & 0xff));
    } else if (u <= 0x7FF) {
        utf8.push_back(static_cast<char>(0xc0 | ((u >> 6) & 0xff)));
        utf8.push_back(static_cast<char>(0x80 | (u & 0x3f)));
    } else if (u <= 0xFFFF) {
        utf8.push_back(static_cast<char>(0xe0 | ((u >> 12) & 0xff)));
        utf8.push_back(static_cast<char>(0x80 | ((u >> 6) & 0x3f)));
        utf8.push_back(static_cast<char>(0x80 | (u & 0x3f)));
    } else {
        assert(u <= 0x10FFFF);
        utf8.push_back(static_cast<char>(0xf0 | ((u >> 18) & 0xff)));
        utf8.push_back(static_cast<char>(0x80 | ((u >> 12) & 0x3f)));
        utf8.push_back(static_cast<char>(0x80 | ((u >> 6) & 0x3f)));
        utf8.push_back(static_cast<char>(0x80 | (u & 0x3f)));
    }
    return utf8;
}

std::string JsonParser::parseRawString() {
    std::string str;
    while (true) {
        switch (*++_curr) {
            case '\"':
                _start = ++_curr;
                return str;
            case '\0':
                throw JsonException("parse miss quotation mark");
            case '\\':
                switch (*++_curr) {
                    case '\"':
                        str.push_back('\"');
                        break;
                    case '\\':
                        str.push_back('\\');
                        break;
                    case '/':
                        str.push_back('/');
                        break;
                    case 'b':
                        str.push_back('\b');
                        break;
                    case 'f':
                        str.push_back('\f');
                        break;
                    case 'n':
                        str.push_back('\n');
                        break;
                    case 't':
                        str.push_back('\t');
                        break;
                    case 'r':
                        str.push_back('\r');
                        break;
                    case 'u': {
                        unsigned u1 = parse4hex();
                        if (u1 >= 0xd800 && u1 <= 0xdbff) {
                            if (*++_curr != '\\') {
                                throw JsonException("parse invalid unicode surrogate");
                            }
                            if (*++_curr != 'u') {
                                throw JsonException("parse invalid unicode surrogate");
                            }
                            unsigned u2 = parse4hex();
                            if (u2 < 0xdc00 || u2 > 0xdfff) {
                                throw(JsonException("parse invalid unicode surrogate"));
                            }
                            u1 = (((u1 - 0xd800) << 10) | (u2 - 0xdc00)) + 0x10000;
                        }
                        str += encodeUTF8(u1);
                    } break;
                    default:
                        throw (JsonException("parse invalid string escape"));
                } break;
            default:
                if (static_cast<unsigned char>(*_curr) < 0x20) {
                    throw(JsonException("parse invalid string char"));
                }
                str.push_back(*_curr);
                break;
        }
    }
}

Json JsonParser::parseString() {
    return Json(parseRawString());
}

Json JsonParser::parseArray() {
    Json::array arr;
    ++_curr;
    parseWhitespace();
    if (*_curr == ']') {
        _start = ++_curr;
        return Json(arr);
    }
    while (true) {
        parseWhitespace();
        arr.push_back(parseValue());
        parseWhitespace();
        if (*_curr == ',') {
            ++_curr;
        } else if (*_curr == ']') {
            _start = ++_curr;
            return(Json(arr));
        } else {
            throw(JsonException("parse miss comma or square bracket"));
        }
    }
}

Json JsonParser::parseObject() {
    Json::object obj;
    ++_curr;
    parseWhitespace();
    if (*_curr == '}') {
        _start = ++_curr;
        return Json(obj);
    }
    while (true) {
        parseWhitespace();
        if (*_curr != '\"') {
            throw(JsonException("parse miss key"));
        }
        std::string key = parseRawString();
        parseWhitespace();
        if (*_curr++ != ':') {
            throw(JsonException("parse miss colon"));
        }
        parseWhitespace();
        Json val = parseValue();
        obj.insert({key, val});
        parseWhitespace();
        if (*_curr == ',') {
            ++_curr;
        } else if (*_curr == '}') {
            _start = ++_curr;
            return Json(obj);
        } else {
            throw JsonException("parse miss comma or curly bracket");
        }
    }
}

Json JsonParser::parse() {
    parseWhitespace();
    Json json = parseValue();
    parseWhitespace();
    if (*_curr) {
        throw(JsonException("parse root not singular"));
    }
    return json;
}