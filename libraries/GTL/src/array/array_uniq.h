#pragma once
#include <inttypes.h>
#include <stddef.h>

namespace gtl {

template <typename T>
class array_uniq {
    T* _buf = nullptr;
    size_t _size = 0;

   public:
    array_uniq(const array_uniq& val) = delete;
    array_uniq& operator=(const array_uniq& val) = delete;

    array_uniq(size_t size = 0) {
        resize(size);
    }

    array_uniq(array_uniq& val) {
        move(val);
    }
    array_uniq& operator=(array_uniq& val) {
        move(val);
        return *this;
    }

#if __cplusplus >= 201103L
    array_uniq(array_uniq&& rval) noexcept {
        move(rval);
    }
    array_uniq& operator=(array_uniq&& rval) noexcept {
        move(rval);
        return *this;
    }
#endif

    ~array_uniq() {
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

    // переместить из другого экземпляра
    void move(array_uniq& val) noexcept {
        if (this == &val) return;
        reset();
        _buf = val._buf;
        _size = val._size;
        val._buf = nullptr;
        val._size = 0;
    }

    // удалить буфер
    void reset() {
        if (_buf) free(_buf);
        _buf = nullptr;
        _size = 0;
    }

    // очистить (заполнить нулями)
    void clear() {
        if (buf()) memset(buf(), 0, size() * sizeof(T));
    }
};

}  // namespace gtl