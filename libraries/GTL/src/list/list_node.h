#pragma once
#include <Arduino.h>

namespace gtl {

class linked_list;

class list_node {
    friend class linked_list;

   public:
    inline list_node* getPrev() {
        return _prev;
    }

   private:
    list_node* _prev = nullptr;
};

}  // namespace gtl