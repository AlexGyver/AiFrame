#pragma once
#include <Arduino.h>

class Reader {
   public:
    Reader(Stream& stream, size_t len) : _stream(&stream), _len(len) {}
    Reader(const uint8_t* bytes, size_t len, bool pgm = false) : _bytes(bytes), _len(len), _pgm(pgm) {}

    int read() {
        if (!_len) return -1;
        _len--;
        
        if (_stream) {
            if (!_waitStream()) return -1;
            _prev = _stream->read();
        } else if (_bytes) {
            _prev = _pgm ? pgm_read_byte(_bytes) : *_bytes;
            _bytes++;
        }
        return _prev;
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

    inline uint8_t prev() {
        return _prev;
    }

    template <typename T>
    bool read(T& val) {
        return read(&val, sizeof(T));
    }

    inline size_t available() {
        return _len;
    }

   private:
    Stream* _stream = nullptr;
    const uint8_t* _bytes = nullptr;
    size_t _len = 0;
    bool _pgm = false;
    uint8_t _prev = 0;

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
