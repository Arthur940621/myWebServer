#include "Coder.h"

unsigned char Coder::hex2dec(char c) {
  if (c >= '0' && c <= '9')
    return c - '0'; // [0-9]
  else if (c >= 'a' && c <= 'z')
    return c - 'a' + 10; // a-f [10-15]
  else if (c >= 'A' && c <= 'Z')
    return c - 'A' + 10; // A-F [10-15]
  return c;
}

char Coder::dec2hex(char c) {
  if (c >= 0 && c <= 9)
    return c + '0'; // ['0'-'9']
  else if (c >= 10 && c <= 15)
    return c - 10 + 'A'; // ['A'-'Z']
  return c;
}

std::string Coder::url_decode(const char *value, size_t size) {
  std::string escaped;
  for (size_t i = 0; i < size; ++i) {
    if (value[i] == '%' && i + 2 < size && isxdigit(value[i + 1]) &&
        isxdigit(value[i + 2])) {
      // merge two char to one byte
      // %AB => 0xAB
      unsigned char byte =
          ((unsigned char)hex2dec(value[i + 1]) << 4) | hex2dec(value[i + 2]);
      i += 2;
      escaped += byte;
    } else {
      escaped += value[i];
    }
  }
  return escaped;
}

std::string Coder::url_decode(const std::string & value) {
  return url_decode(value.data(), value.size());
}

std::string Coder::url_decode(const char *value) {
  return url_decode(value, strlen(value));
}

std::string Coder::url_encode(const char *value, size_t size) {
  std::string escaped;

  for (size_t i = 0; i < size; i++) {
    unsigned char c = (unsigned char)value[i];
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      escaped += c;
      continue;
    }
    // split one byte to two char
    // 0xAB => %AB
    char buf[5]{0};
    sprintf(buf, "%%%c%c", toupper(dec2hex(c >> 4)), toupper(dec2hex(c & 15)));
    escaped += buf;
  }

  return escaped;
}

std::string Coder::url_encode(const std::string & value) {
  return url_encode(value.data(), value.size());
}

std::string Coder::url_encode(const char *value) {
  return url_encode(value, strlen(value));
}