#include <Arduino.h>
#include <GTL.h>

// наследуем list_node
class Test : public gtl::list_node {
   public:
    Test(int i) : i(i) {}

    // переменная для примера
    int i;
};

void setup() {
    Serial.begin(115200);
    Serial.println("start");

    Test t0(0), t1(1), t2(2);

    gtl::linked_list list;
    // добавляем в список
    list.add(t0);
    list.add(t1);
    list.add(t2);

    // длина списка
    Serial.print("length: ");
    Serial.println(list.length());

    // пробегаемся по списку
    gtl::list_iter iter = list.iter();
    Test* p;
    while ((p = (Test*)iter.next())) {
        // и выводим значения
        Serial.println(p->i);
    }

    // или так
    gtl::list_node* node = list.getLast();
    while (node) {
        // node...
        node = node->getPrev();
    }
}

void loop() {
}