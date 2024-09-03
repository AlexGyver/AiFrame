#pragma once
#include <Arduino.h>

class Reader {
   public:
    Reader(Stream& stream, size_t len) : _stream(&stream), _len(len) {}
    Reader(const uint8_t* bytes, size_t len, bool pgm = false) : _bytes(bytes), _len(len), _pgm(pgm) {}

    int peek() {
        if (!_len) return -1;
        if (_stream) {
            if (!_waitStream()) return -1;
            return _stream->peek();
        } else if (_bytes) {
            return _pgm ? pgm_read_byte(_bytes) : *_bytes;
        }
    }

    int read() {
        if (!_len) return -1;
        _prev = _buf;
        _len--;
        if (_stream) {
            if (!_waitStream()) return -1;
            _buf = _stream->read();
        } else if (_bytes) {
            _buf = _pgm ? pgm_read_byte(_bytes) : *_bytes;
            _bytes++;
        }
        return _buf;
    }

    bool read(void* dest, size_t size) {
        if (size > _len) return 0;
        _len -= size;

        if (_stream) {
            return (_stream->readBytes((uint8_t*)dest, size) == size);
        } else if (_bytes) {
            _pgm ? memcpy_P(dest, _bytes, size) : memcpy(dest, _bytes, size);
            _bytes += size;
            return 1;
        }
        return 0;
    }

    // предыдущий прочитанный символ
    inline uint8_t prev() {
        return _prev;
    }

    // текущий прочитанный символ после read()
    inline uint8_t current() {
        return _buf;
    }

    template <typename T>
    bool read(T& val) {
        return read(&val, sizeof(T));
    }

    inline size_t available() {
        return _len;
    }

    void setLength(size_t len) {
        _len = len;
    }

   private:
    Stream* _stream = nullptr;
    const uint8_t* _bytes = nullptr;
    size_t _len = 0;
    bool _pgm = false;
    uint8_t _prev = 0, _buf = 0;

    bool _waitStream() {
        if (!_stream->available()) {
            uint16_t i = 0;
            while (!_stream->available()) {
                delay(1);
                if (++i == _stream->getTimeout()) {
                    _len = 0;
                    return 0;
                }
            }
        }
        return 1;
    }
};
