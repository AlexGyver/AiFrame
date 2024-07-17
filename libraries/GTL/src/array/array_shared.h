#pragma once
#include <inttypes.h>
#include <stddef.h>

namespace gtl {

template <typename T>
class array_shared {
    struct shared_data {
        T* bufp = nullptr;
        size_t size = 0;
        uint16_t count = 1;
    };

    shared_data* _data = nullptr;

   public:
    array_shared(size_t size = 0) {
        resize(size);
    }

    array_shared(const array_shared& val) {
        share(val);
    }
    array_shared& operator=(const array_shared& val) {
        share(val);
        return *this;
    }

#if __cplusplus >= 201103L
    array_shared(array_shared&& rval) noexcept {
        share(rval);
    }
    array_shared& operator=(array_shared&& rval) noexcept {
        share(rval);
        return *this;
    }
#endif

    ~array_shared() {
        release();
    }

    uint16_t getCount() const {
        return _data ? _data->count : 0;
    }
    T* buf() const {
        return _data ? _data->bufp : nullptr;
    }
    size_t size() const {
        return buf() ? _data->size : 0;
    }
    operator T*() const {
        return buf();
    }
    explicit operator bool() const {
        return buf();
    }

    // изменить размер в количестве элементов T
    bool resize(size_t newsize) {
        if (!_data) {
            _data = new shared_data;
            if (!_data) return 0;
        }
        if (size() >= newsize) return 1;

        T* newb = (T*)realloc(buf(), newsize * sizeof(T));
        if (!newb) return 0;
        _data->bufp = newb;
        _data->size = newsize;
        return 1;
    }
    void share(const array_shared& val) noexcept {
        if (this == &val) return;
        if (_data == val._data) return;

        if (_data) {
            release();
            _data = val._data;
            if (_data) _data->count++;
        } else {
            if (val._data) {
                _data = val._data;
                _data->count++;
            } else {
                // ==> _data == val._data
            }
        }
    }

    // получить отвязанную копию экземпляра
    array_shared copy() const {
        if (buf()) {
            array_shared t;
            if (t.resize(size())) {
                memcpy((void*)t.buf(), (void*)buf(), size() * sizeof(T));
            }
            return t;
        }
        return array_shared();
    }

    // удалить буфер
    void reset() {
        if (_data) {
            _data->size = 0;
            free(_data->bufp);
            _data->bufp = nullptr;
        }
    }

    // очистить (заполнить нулями)
    void clear() {
        if (buf()) memset(buf(), 0, size() * sizeof(T));
    }

    // освободить экземпляр, удалить буфер если экземпляр был последний
    void release() {
        if (_data && --(_data->count) == 0) {
            free(_data->bufp);
            delete _data;
        }
        _data = nullptr;
    }
};

}  // namespace gtl