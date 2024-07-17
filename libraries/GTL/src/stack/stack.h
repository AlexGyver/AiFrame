#pragma once
#include "array/array_copy.h"
#include "array/array_shared.h"
#include "array/array_uniq.h"
#include "stack_ext.h"

namespace gtl {

// stackT
template <typename T, template <typename> class array_t>
class stackT : public stack_ext<T> {
   public:
    // зарезервировать память в кол-ве элементов
    stackT() {}
    stackT(size_t size) {
        reserve(size);
    }

    stackT(const stack_ext<T>& st) = delete;
    stackT(stack_ext<T>&& st) = delete;
    stackT& operator=(const stack_ext<T>& st) = delete;
    stackT& operator=(stack_ext<T>&& st) = delete;

    stackT(const stackT<T, array_t>& st) {
        if (this == &st) return;
        _array = st._array;
        _len = st._len;
        _update();
    }
    stackT& operator=(const stackT<T, array_t>& st) {
        if (this == &st) return *this;
        _array = st._array;
        _len = st._len;
        _update();
        return *this;
    }

#if __cplusplus >= 201103L
    stackT(stackT<T, array_t>&& st) noexcept {
        _array = st._array;
        _len = st._len;
        _update();
    }
    stackT& operator=(stackT<T, array_t>&& st) noexcept {
        _array = st._array;
        _len = st._len;
        _update();
        return *this;
    }
#endif

    // зарезервировать память в кол-ве элементов
    bool reserve(size_t cap) {
        bool ok = _array.resize(cap);
        _update();
        return ok;
    }

    // удалить буфер
    void reset() {
        _array.reset();
        _update();
    }

    // есть место для добавления (всегда true)
    bool canAdd() {
        return 1;
    }

   protected:
    array_t<T> _array;
    using stack_ext<T>::setBuffer;
    using stack_ext<T>::_buf;
    using stack_ext<T>::_cap;
    using stack_ext<T>::_len;

    void _update() {
        _buf = _array.buf();
        _cap = _array.size();
    }
};

// stack_copy
template <typename T>
class stack_copy : public stackT<T, array_copy> {
    using stackT<T, array_copy>::_update;
    using stackT<T, array_copy>::_array;

   protected:
    using stackT<T, array_copy>::_len;

   public:
    stack_copy() {}
    using stackT<T, array_copy>::stackT;

    void move(stack_copy<T>& st) {
        if (this == &st) return;
        _array = st._array;
        _len = st._len;
        _update();
    }
};

// stack_shared
template <typename T>
class stack_shared : public stackT<T, array_shared> {
   public:
    using stackT<T, array_shared>::stackT;
    stack_shared() {}
};

// stack_uniq
template <typename T>
class stack_uniq : public stackT<T, array_uniq> {
    using stackT<T, array_uniq>::_array;
    using stackT<T, array_uniq>::_update;

   protected:
    using stackT<T, array_uniq>::_len;

   public:
    stack_uniq() {}
    using stackT<T, array_uniq>::stackT;

    stack_uniq(const stack_uniq<T>& st) = delete;
    stack_uniq& operator=(const stack_uniq<T>& st) = delete;

    stack_uniq(stack_uniq<T>& st) {
        move(st);
    }
    stack_uniq& operator=(stack_uniq<T>& st) {
        move(st);
        return *this;
    }

    void move(stack_uniq<T>& st) noexcept {
        if (this == &st) return;
        _array = st._array;
        _len = st._len;
        st._update();
        _update();
    }
};

}  // namespace gtl