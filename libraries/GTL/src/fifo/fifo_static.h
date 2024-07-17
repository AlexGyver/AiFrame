#pragma once
#include "fifo_ext.h"

namespace gtl {

// FIFO Static
template <typename T, uint8_t size>
class fifo_static : public fifo_ext<T> {
   public:
    fifo_static() : fifo_ext<T>(_buffer, size + 1) {}

   private:
    using fifo_ext<T>::setBuffer;
    T _buffer[size + 1];
};

}  // namespace gtl