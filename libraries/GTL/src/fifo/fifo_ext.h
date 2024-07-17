#pragma once
#include <inttypes.h>
#include <stddef.h>

namespace gtl {

// FIFO External
template <typename T>
class fifo_ext {
   public:
    fifo_ext(T* buf = nullptr, uint8_t capacity = 0) {
        setBuffer(buf, capacity);
    }

    // подключить буфер
    void setBuffer(T* buf, uint8_t capacity) {
        buffer = buf;
        _cap = capacity;
    }

    // запись в буфер. Вернёт true при успешной записи
    bool write(const T& val) {
        if (buffer) {
            uint8_t next = _leap(_head);
            if (next != _tail) {
                buffer[_head] = val;
                _head = next;
                return 1;
            }
        }
        return 0;
    }

    // буфер полон
    bool isFull() const {
        return _leap(_head) == _tail;
    }

    // буфер пуст
    inline bool isEmpty() const {
        return _head == _tail;
    }

    // чтение из буфера
    T read() {
        if (!buffer || isEmpty()) return T();
        uint8_t t = _tail;
        _tail = _leap(_tail);
        return buffer[t];
    }

    // возвращает крайнее значение без удаления из буфера
    T peek() {
        return (!buffer || isEmpty()) ? T() : buffer[_tail];
    }

    // количество непрочитанных элементов
    uint8_t available() {
        return buffer ? ((_head >= _tail) ? (_head - _tail) : (_cap + _head - _tail)) : 0;
    }

    // размер буфера
    uint8_t size() const {
        return (buffer && _cap) ? (_cap - 1) : 0;
    }

    // очистить
    void clear() {
        _head = _tail = 0;
    }

    T* buffer = nullptr;

   private:
    uint8_t _cap = 0, _head = 0, _tail = 0;

    inline uint8_t _leap(const uint8_t pos) const {
        return (pos + 1) >= _cap ? 0 : (pos + 1);
    }
};

}  // namespace gtl