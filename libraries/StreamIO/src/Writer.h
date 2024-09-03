#pragma once
#include <Arduino.h>

class Writer {
   public:
    Writer(Stream& stream) : _stream(&stream) {}
    Writer(uint8_t* buffer) : _buffer(buffer) {}

    size_t write(const uint8_t* data, size_t size) {
        if (_stream) {
            size_t w = _stream->write((const uint8_t*)data, size);
            _writed += w;
            return w;
        } else if (_buffer) {
            memcpy(_buffer, data, size);
            _buffer += size;
            _writed += size;
            return size;
        }
        return 0;
    }

    bool write(const void* data, size_t size) {
        return write((const uint8_t*)data, size) == size;
    }

    template <typename T>
    bool write(const T& val) {
        return write((const void*)&val, sizeof(T));
    }

    size_t writed() {
        return _writed;
    }

   private:
    Stream* _stream = nullptr;
    uint8_t* _buffer = nullptr;
    size_t _writed = 0;
};