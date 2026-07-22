#pragma once
#include <Arduino.h>
#include <StringUtils.h>

class StreamB64 {
   public:
    StreamB64(Stream& stream, size_t bufsize = 512) : stream(stream), bufsize(bufsize) {
        buffer = new uint8_t[bufsize];
        stream.setTimeout(500);
    }
    ~StreamB64() {
        delete[] buffer;
    }

    void readBytes(uint8_t* buf, size_t len) {
        while (len--) {
            uint8_t b = getByte();
            if (buf) *buf++ = b;
        }
    }

   private:
    Stream& stream;
    size_t bufsize;

    uint8_t* buffer = nullptr;
    uint8_t* bufptr = nullptr;
    size_t bufleft = 0;

    uint32_t val = 0;
    int8_t valb = -8;

    uint8_t getByte() {
        uint8_t b = 0;
        while (valb < 0) {
            char c = 0;

            if (buffer) {
                if (!bufleft) {
                    bufleft = stream.readBytes(buffer, bufsize);
                    bufptr = buffer;
                }
                c = *bufptr;
                bufptr++;
                bufleft--;
            }

            if (c == '=') return b;
            val = (val << 6) + su::b64::getByte(c);
            valb += 6;
            if (valb >= 0) {
                b = (val >> valb) & 0xFF;
                valb -= 8;
                break;
            }
        }
        return b;
    }
};