#pragma once
#include "stack_ext.h"

namespace gtl {

// Static Stack
template <typename T, size_t size>
class stack_static : public stack_ext<T> {
   public:
    stack_static() : stack_ext<T>(_buffer, size) {}

   private:
    using stack_ext<T>::setBuffer;
    T _buffer[size];
};

}  // namespace gtl