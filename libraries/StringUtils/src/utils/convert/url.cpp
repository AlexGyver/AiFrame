#include "url.h"

namespace su {
namespace url {

// символ должен быть urlencoded
bool needsEncode(char c) {
    switch (c) {
        case '0' ... '9':
        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '.':
        case '-':
        case '_':
        case '~':
            // case '!': case '+': case ',':
            // case '*': case ':': case '@':
            return 0;
    }
    return 1;
}

// закодировать в url. Можно указать len = 0, если неизвестна
void encode(const char* src, uint16_t len, String& dest) {
    if (!len) len = strlen(src);
    dest.reserve(len);
    const char* end = src + len;
    while (src < end) {
        char c = *src++;
        if (needsEncode(c)) {
            dest += '%';
            dest += (char)((c >> 4) + (((c >> 4) > 9) ? 55 : '0'));
            dest += (char)((c & 0xF) + (((c & 0xF) > 9) ? 55 : '0'));
        } else {
            dest += c;
        }
    }
}

// закодировать в url
void encode(const String& src, String& dest) {
    encode(src.c_str(), src.length(), dest);
}

// закодировать в url
String encode(const String& src) {
    String dest;
    encode(src, dest);
    return dest;
}

static uint8_t _decodeNibble(char c) {
    return c - ((c <= '9') ? '0' : ((c <= 'F') ? 55 : 87));
}

// раскодировать url
void decode(const char* src, uint16_t len, String& dest) {
    if (!len) len = strlen(src);
    dest.reserve(len);
    const char* end = src + len;
    while (src < end) {
        char c = *src++;
        if (c != '%') {
            dest += (c == '+') ? ' ' : c;
        } else {
            if (end - src < 2) return;
            char c1 = *src++;
            char c2 = *src++;
            dest += char(_decodeNibble(c2) | (_decodeNibble(c1) << 4));
        }
    }
}

// раскодировать url
size_t decode(char* dest, const char* url, uint16_t len) {
    if (!len) len = strlen(url);
    char* out = dest;
    const char* end = url + len;
    while (url < end) {
        char c = *url++;
        if (c != '%') {
            *out++ = (c == '+') ? ' ' : c;
        } else {
            if (end - url < 2) break;
            char c1 = *url++;
            char c2 = *url++;
            *out++ = char(_decodeNibble(c2) | (_decodeNibble(c1) << 4));
        }
    }
    if (url <= end) *out = 0;
    return out - dest;
}

// раскодировать url саму в себя
size_t decode(char* url, uint16_t len) {
    return decode(url, url, len);
}

// раскодировать url
void decode(const String& src, String& dest) {
    decode(src.c_str(), src.length(), dest);
}

// раскодировать url
String decode(const String& src) {
    String dest;
    decode(src.c_str(), src.length(), dest);
    return dest;
}

// раскодировать url
String decode(const char* src, uint16_t len) {
    String dest;
    decode(src, len, dest);
    return dest;
}

}  // namespace url
}  // namespace su