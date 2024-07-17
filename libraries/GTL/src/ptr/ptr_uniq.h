#pragma once
#include <inttypes.h>
#include <stddef.h>

namespace gtl {

template <typename T>
class ptr_uniq {
    T* _ptr = nullptr;

   public:
    ptr_uniq(const ptr_uniq& val) = delete;
    ptr_uniq& operator=(const ptr_uniq& val) = delete;

    explicit ptr_uniq(T* ptr) : _ptr(ptr) {}

    ptr_uniq(ptr_uniq& val) {
        move(val);
    }
    ptr_uniq& operator=(ptr_uniq& val) {
        move(val);
        return *this;
    }

#if __cplusplus >= 201103L
    ptr_uniq(ptr_uniq&& rval) noexcept {
        move(rval);
    }
    ptr_uniq& operator=(ptr_uniq&& rval) noexcept {
        move(rval);
        return *this;
    }
#endif

    ~ptr_uniq() {
        reset();
    }

    T* get() const {
        return _ptr;
    }
    T* operator->() const {
        return _ptr;
    }
    T& operator*() const {
        return *_ptr;
    }
    explicit operator bool() const {
        return _ptr;
    }

    // переместить из другого экземпляра
    void move(ptr_uniq& val) noexcept {
        if (this == &val) return;
        reset();
        _ptr = val._ptr;
        val._ptr = nullptr;
    }

    // удалить
    void reset() {
        if (_ptr) delete _ptr;
        _ptr = nullptr;
    }
};

}  // namespace gtl