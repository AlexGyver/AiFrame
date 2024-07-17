#pragma once
#include <Arduino.h>

class Writer {
   public:
    Writer(Stream& stream) : _stream(&stream) {}
    Writer(uint8_t* buffer) : _buffer(buffer) {}

    bool write(const void* data, size_t size) {
        if (_stream) {
            size_t w = _stream->write((const uint8_t*)data, size);
            _writed += w;
            return w == size;
        } else if (_buffer) {
            memcpy(_buffer, data, size);
            _buffer += size;
            _writed += size;
            return 1;
        }
        return 0;
    }

    template <typename T>
    bool write(T& val) {
        return write(&val, sizeof(T));
    }

    size_t writed() {
        return _writed;
    }

   private:
    Stream* _stream = nullptr;
    uint8_t* _buffer = nullptr;
    size_t _writed = 0;
};