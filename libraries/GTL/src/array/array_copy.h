#pragma once
#include <inttypes.h>
#include <stddef.h>

namespace gtl {

template <typename T>
class array_copy {
    T* _buf = nullptr;
    size_t _size = 0;

   public:
    array_copy(size_t size = 0) {
        resize(size);
    }

    array_copy(const array_copy& val) {
        copy(val);
    }
    array_copy& operator=(const array_copy& val) {
        copy(val);
        return *this;
    }

#if __cplusplus >= 201103L
    array_copy(array_copy&& rval) noexcept {
        move(rval);
    }
    array_copy& operator=(array_copy&& rval) noexcept {
        move(rval);
        return *this;
    }
#endif

    ~array_copy() {
        reset();
    }

    T* buf() const {
        return _buf;
    }
    size_t size() const {
        return buf() ? _size : 0;
    }
    operator T*() const {
        return buf();
    }
    explicit operator bool() const {
        return buf();
    }

    // изменить размер в количестве элементов T
    bool resize(size_t newsize) {
        if (_size >= newsize) return 1;

        T* newb = (T*)realloc(_buf, newsize * sizeof(T));
        if (!newb) return 0;
        _buf = newb;
        _size = newsize;
        return 1;
    }

    // копировать из другого экземпляра
    bool copy(const array_copy& val) {
        if (this == &val) return 1;
        if (!val._buf || !resize(val._size)) {
            reset();
            return 0;
        }
        memcpy((void*)_buf, (void*)val._buf, val._size * sizeof(T));
        return 1;
    }

    // переместить из другого экземпляра
    void move(array_copy& rval) noexcept {
        if (this == &rval) return;
        if (_buf) {
            if (rval._buf && _size >= rval._size) {
                memcpy((void*)_buf, (void*)rval._buf, rval._size * sizeof(T));
                return;
            } else {
                free(_buf);
            }
        }
        _buf = rval._buf;
        _size = rval._size;
        rval._buf = nullptr;
    }

    // удалить буфер
    void reset() {
        free(_buf);
        _buf = nullptr;
        _size = 0;
    }

    // очистить (заполнить нулями)
    void clear() {
        if (buf()) memset(buf(), 0, size() * sizeof(T));
    }
};

}  // namespace gtl