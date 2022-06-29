#pragma once

#include <functional>
#include <cstring>

struct Coder {
  static unsigned char hex2dec(char);
  static char dec2hex(char);

  static std::string url_decode(const char *, size_t);
  static std::string url_decode(const std::string &);
  static std::string url_decode(const char *);

  static std::string url_encode(const char *, size_t);
  static std::string url_encode(const std::string &);
  static std::string url_encode(const char *);
};