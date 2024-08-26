#pragma once
#include <Arduino.h>
#include <StringUtils.h>

#include "utils/cfg.h"

#define READER_LENSTR_LEN 10
#define READER_DEF_TOUT 500

// ==================== READER ====================
class StreamReader : public Printable, public Stream {
    class WritableString : public String {
       public:
        size_t write(uint8_t* data, size_t len) {
            return concat((char*)data, len) ? len : 0;
        }
    };

   public:
    class Buffer {
       public:
        size_t write(uint8_t* data, size_t len) {
            return s.concat((char*)data, len) ? len : 0;
        }

        uint8_t* buf() {
            return (uint8_t*)s.c_str();
        }
        size_t length() {
            return s.length();
        }

       private:
        String s;
    };

    StreamReader(Stream* stream = nullptr, size_t len = 0, bool chunked = false) : stream(stream), _len(len), _chunked(chunked), _tout(stream ? stream->getTimeout() : READER_DEF_TOUT) {}

    // установить размер блока
    void setBlockSize(size_t bsize) {
        _bsize = bsize;
    }

    void setTimeout(size_t tout) {
        _tout = tout;
        if (stream) stream->setTimeout(tout);
    }

    // http chunked response
    bool isChunked() {
        return _chunked;
    }

    // корреткность ридера
    operator bool() {
        return available();
    }

    // оставшийся размер входящих данных. 1 если chunked
    size_t length() {
        return available();
    }

    // override
    size_t write(uint8_t) { return 0; }

    int available() {
        return stream ? (_chunked ? 1 : _len) : 0;
    }

    int read() {
        if (!available()) return -1;
        char c;
        readBytes(&c, 1);
        return c;
    }

    int peek() {
        return '0';
    }

    Buffer readBuffer() {
        Buffer b;
        writeTo(b);
        return b;
    }

    bool readBuffer(Buffer& b) {
        return writeTo(b);
    }

    String readString() {
        WritableString s;
        writeTo(s);
        return s;
    }

    size_t readBytes(char* buffer, size_t length) {
        if (!stream) return 0;

        if (_chunked) {
            size_t wasread = 0;
            while (length) {
                GHTTP_ESP_YIELD();
                if (_chunklen) {
                    size_t curlen = min(_chunklen, length);
                    size_t read = stream->readBytes(buffer, curlen);
                    wasread += read;
                    buffer += curlen;
                    length -= curlen;
                    _chunklen -= curlen;

                    if (read != curlen) {  // read error
                        stream = nullptr;
                        break;
                    }

                    if (!_chunklen) {
                        if (!_endChunk()) {  // chunked end error
                            stream = nullptr;
                            break;
                        }
                    }

                } else {
                    int chlen = _readChunkLen();
                    if (chlen <= 0) {
                        if (chlen < 0) {
                            // read len error
                        }
                        if (!chlen) {
                            if (!_endChunk()) {
                                // chunked end error
                            }
                        }
                        stream = nullptr;
                        break;
                    }
                    _chunklen = chlen;
                }
            }
            return wasread;

        } else {
            if (length > _len) length = _len;
            _len -= length;
            size_t read = stream->readBytes(buffer, length);
            if (!_len) stream = nullptr;
            return read;
        }
        return 0;
    }

    // вывести всё в write(uint8_t*, size_t). Вернёт количество записанных или 0 при ошибке
    template <typename T>
    size_t writeTo(T& p) const {
        return _chunked ? _writeTo(p) : (_writeTo(p) == _len ? _len : 0);
    }

    // вывести всё в Print
    size_t printTo(Print& p) const {
        return _writeTo(p);
    }

    Stream* stream = nullptr;

   private:
    size_t _len;
    size_t _bsize = 128;
    bool _chunked = false;
    size_t _chunklen = 0;
    size_t _tout;

    // -1 error
    int _readChunkLen() {
        char lenstr[READER_LENSTR_LEN];
        size_t len = stream->readBytesUntil('\n', lenstr, READER_LENSTR_LEN);
        if (len < 2 || lenstr[len - 1] != '\r') return -1;
        return su::strToIntHex(lenstr, len - 1);
    }
    bool _endChunk() {
        char r, n;
        stream->readBytes(&r, 1);
        stream->readBytes(&n, 1);
        return (r == '\r' && n == '\n');
    }

    template <typename T>
    size_t _writeTo(T& p) const {
        if (!stream) return 0;
        uint8_t* buf = new uint8_t[_chunked ? _bsize : min(_bsize, _len)];
        if (!buf) return 0;

        size_t n = 0;
        if (_chunked) {
            char lenstr[READER_LENSTR_LEN];
            while (1) {
                GHTTP_ESP_YIELD();

                bool last = 0;
                size_t len = stream->readBytesUntil('\n', lenstr, READER_LENSTR_LEN);
                if (!len || lenstr[len - 1] != '\r') {
                    n = 0;
                    break;
                }

                len = su::strToIntHex(lenstr, len - 1);
                if (len) {
                    size_t w = _writeBuffered(len, buf, p);
                    if (w != len) {
                        n = 0;
                        break;
                    }
                    n += w;
                } else {
                    last = 1;
                }

                len = stream->readBytesUntil('\n', lenstr, READER_LENSTR_LEN);
                if (len != 1 || lenstr[0] != '\r') {
                    n = 0;
                    break;
                }

                if (last) break;
            }

        } else {
            if (_len) n = _writeBuffered(_len, buf, p);
        }

        delete[] buf;
        return n;
    }

    template <typename T>
    size_t _writeBuffered(size_t len, uint8_t* buffer, T& p) const {
        size_t left = len;
        while (left) {
            GHTTP_ESP_YIELD();
            if (!_waitStream()) break;

            size_t block = min(min(left, (size_t)stream->available()), _bsize);
            size_t read = stream->readBytes(buffer, block);
            GHTTP_ESP_YIELD();

            if (read != block) break;
            if (block != p.write(buffer, block)) break;
            left -= block;
        }
        return len - left;
    }

    bool _waitStream() const {
        if (!stream->available()) {
            int ms = _tout;
            while (!stream->available()) {
                delay(1);
                if (!--ms) return 0;
            }
        }
        return 1;
    }
};