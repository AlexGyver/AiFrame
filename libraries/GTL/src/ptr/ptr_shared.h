#pragma once
#include <inttypes.h>
#include <stddef.h>

namespace gtl {

template <typename T>
class ptr_shared {
    struct shared_data {
        T* ptr = nullptr;
        uint16_t count = 1;
    };

    shared_data* _data = nullptr;

   public:
    explicit ptr_shared(T* ptr) {
        _data = new shared_data;
        if (_data) _data->ptr = ptr;
    }

    ptr_shared(const ptr_shared& val) {
        share(val);
    }
    ptr_shared& operator=(const ptr_shared& val) {
        share(val);
        return *this;
    }

#if __cplusplus >= 201103L
    ptr_shared(ptr_shared&& rval) noexcept {
        share(rval);
    }
    ptr_shared& operator=(ptr_shared&& rval) noexcept {
        share(rval);
        return *this;
    }
#endif

    ~ptr_shared() {
        release();
    }

    uint16_t getCount() const {
        return _data ? _data->count : 0;
    }
    T* get() const {
        return _data ? _data->ptr : nullptr;
    }
    T* operator->() const {
        return get();
    }
    T& operator*() const {
        return *get();
    }
    explicit operator bool() const {
        return get();
    }

    void share(const ptr_shared& val) noexcept {
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

    // удалить буфер
    void reset() {
        if (_data) {
            delete _data->ptr;
            _data->ptr = nullptr;
        }
    }

    // освободить экземпляр, удалить буфер если экземпляр был последний
    void release() {
        if (_data && --(_data->count) == 0) {
            delete _data->ptr;
            delete _data;
        }
        _data = nullptr;
    }
};

}  // namespace gtl