[![latest](https://img.shields.io/github/v/release/GyverLibs/GTL.svg?color=brightgreen)](https://github.com/GyverLibs/GTL/releases/latest/download/GTL.zip)
[![PIO](https://badges.registry.platformio.org/packages/gyverlibs/library/GTL.svg)](https://registry.platformio.org/libraries/gyverlibs/GTL)
[![Foo](https://img.shields.io/badge/Website-AlexGyver.ru-blue.svg?style=flat-square)](https://alexgyver.ru/)
[![Foo](https://img.shields.io/badge/%E2%82%BD%24%E2%82%AC%20%D0%9F%D0%BE%D0%B4%D0%B4%D0%B5%D1%80%D0%B6%D0%B0%D1%82%D1%8C-%D0%B0%D0%B2%D1%82%D0%BE%D1%80%D0%B0-orange.svg?style=flat-square)](https://alexgyver.ru/support_alex/)
[![Foo](https://img.shields.io/badge/README-ENGLISH-blueviolet.svg?style=flat-square)](https://github-com.translate.goog/GyverLibs/GTL?_x_tr_sl=ru&_x_tr_tl=en)  

[![Foo](https://img.shields.io/badge/ПОДПИСАТЬСЯ-НА%20ОБНОВЛЕНИЯ-brightgreen.svg?style=social&logo=telegram&color=blue)](https://t.me/GyverLibs)

# Gyver Template Library
Набор шаблонных инструментов
- Умные указатели
- Динамические массивы
- Буферы

### Совместимость
Совместима со всеми Arduino платформами (используются Arduino-функции)

## Содержание
- [Использование](#usage)
- [Версии](#versions)
- [Установка](#install)
- [Баги и обратная связь](#feedback)

<a id="usage"></a>

## Использование
### ВНИМАНИЕ!
`array_x`/`stack_x` используют сишный аллокатор `realloc` для изменения размера. Это позволяет менять размер без создания дыр в памяти, как в других vector-подобных библиотеках, память используется очень сильно эффективнее. Но в то же время **GTL-массивы не вызывают конструкторы и деструкторы**, поэтому категорически не рекомендуется использовать их с объектами, которые содержат динамические данные (например String) или copy/move семантику!!! 

### gtl::array_x
Динамический массив. Имеет три модели поведения при передаче из одного объекта в другой:
- `array_copy` - создаёт независимую копию (как Arduino String)
- `array_shared` - создаёт зависимую копию, изменения в одном массиве отражаются на другом
- `array_uniq` - полностью переходит к новому объекту

```cpp
T* buf();
size_t size();
operator T*();
explicit operator bool();

// изменить размер в количестве элементов T
bool resize(size_t newsize);

// удалить буфер
void reset();
```

### gtl::stack_x
Динамический массив, хранит данные линейно, позволяет добавлять и убирать их. Имеет несколько вариантов:
- `stack_ext` - внешний буфер
- `stack_static` - внутренний статический буфер
- `stack_copy` - динамический буфер с семантикой `array_copy`
- `stack_shared` - динамический буфер с семантикой `array_shared`
- `stack_uniq` - динамический буфер с семантикой `array_uniq`

```cpp
// добавить в конец
bool push(const T& val);

// добавить в конец
bool operator+=(const T& val);

// получить с конца и удалить
T pop();

// прочитать с конца не удаляя
T& peek();

// добавить в начало
bool shift(const T& val);

// получить с начала и удалить
T unshift();

// прочитать с начала не удаляя
T& unpeek();

// удалить элемент. Отрицательный - с конца
bool remove(int idx);

// удалить несколько элементов, начиная с индекса
bool remove(size_t from, size_t amount);

// вставить элемент на индекс (допускается индекс length())
bool insert(int idx, const T& val);

// прибавить другой массив в конец
bool concat(const stack_ext& st);
bool concat(const T* buf, size_t len, bool pgm = false);

// прибавить другой массив в конец
bool operator+=(const stack_ext& st);

// заполнить значением (на capacity)
void fill(const T& val);

// инициализировать, вызвать конструкторы (на capacity)
void init();

// очистить (установить длину 0)
void clear();

// текущий размер в байтах
size_t size();

// количество элементов
size_t length();

// установить количество элементов (само вызовет reserve)
bool setLength(size_t len);

// есть место для добавления
bool canAdd();

// вместимость, элементов
size_t capacity();

// позиция элемента (-1 если не найден)
int indexOf(const T& val);

// содержит элемент
bool has(const T& val);

// удалить по значению (true если элемента нет)
bool removeByVal(const T& val);

// получить элемент под индексом. Отрицательный - с конца
T& get(int idx);

// получить элемент под индексом. Отрицательный - с конца
T& operator[](int idx);

// доступ к буферу
inline T* buf();

// буфер существует
bool valid();

// буфер существует
explicit operator bool();

// бинарный поиск в отсортированном стеке
bsearch_t<T> searchSort(const T& val);

// добавить с сортировкой. Флаг uniq - не добавлять если элемент уже есть
bool addSort(const T& val, bool uniq = false);

// ДЛЯ ДИНАМИЧЕСКИХ
// зарезервировать память в кол-ве элементов
bool reserve(size_t cap);

// удалить буфер
void reset();
```

### gtl::fifo_x
FIFO буфер
- `fifo_ext` - внешний буфер
- `fifo_static` - внутренний статический буфер

```cpp
// подключить буфер
void setBuffer(T* buf, uint8_t capacity);

// запись в буфер. Вернёт true при успешной записи
bool write(const T& val);

// буфер полон
bool isFull();

// буфер пуст
inline bool isEmpty();

// чтение из буфера
T read();

// возвращает крайнее значение без удаления из буфера
T peek();

// количество непрочитанных элементов
uint8_t available();

// размер буфера
uint8_t size();

// очистить
void clear();

T* buffer;
```

### gtl::linked_list
Связанный список для создания динамического массива объектов в стеке. Смотри пример в examples
```cpp
// получить итератор
list_iter iter();

// добавить
bool add(list_node& node);

// добавить
bool add(list_node* node);

// удалить
void remove(list_node& node);

// удалить
void remove(list_node* node);

// список содержит
bool has(list_node& node);

// список содержит
bool has(list_node* node);

// длина списка
size_t length();

// очистить список
void clear();

// получить последний элемент в списке
list_node* getLast();
```

<a id="versions"></a>

## Версии
- v1.0
- v1.1 - добавлен связанный список

<a id="install"></a>
## Установка
- Библиотеку можно найти по названию **GTL** и установить через менеджер библиотек в:
    - Arduino IDE
    - Arduino IDE v2
    - PlatformIO
- [Скачать библиотеку](https://github.com/GyverLibs/GTL/archive/refs/heads/main.zip) .zip архивом для ручной установки:
    - Распаковать и положить в *C:\Program Files (x86)\Arduino\libraries* (Windows x64)
    - Распаковать и положить в *C:\Program Files\Arduino\libraries* (Windows x32)
    - Распаковать и положить в *Документы/Arduino/libraries/*
    - (Arduino IDE) автоматическая установка из .zip: *Скетч/Подключить библиотеку/Добавить .ZIP библиотеку…* и указать скачанный архив
- Читай более подробную инструкцию по установке библиотек [здесь](https://alexgyver.ru/arduino-first/#%D0%A3%D1%81%D1%82%D0%B0%D0%BD%D0%BE%D0%B2%D0%BA%D0%B0_%D0%B1%D0%B8%D0%B1%D0%BB%D0%B8%D0%BE%D1%82%D0%B5%D0%BA)
### Обновление
- Рекомендую всегда обновлять библиотеку: в новых версиях исправляются ошибки и баги, а также проводится оптимизация и добавляются новые фичи
- Через менеджер библиотек IDE: найти библиотеку как при установке и нажать "Обновить"
- Вручную: **удалить папку со старой версией**, а затем положить на её место новую. "Замену" делать нельзя: иногда в новых версиях удаляются файлы, которые останутся при замене и могут привести к ошибкам!

<a id="feedback"></a>

## Баги и обратная связь
При нахождении багов создавайте **Issue**, а лучше сразу пишите на почту [alex@alexgyver.ru](mailto:alex@alexgyver.ru)  
Библиотека открыта для доработки и ваших **Pull Request**'ов!

При сообщении о багах или некорректной работе библиотеки нужно обязательно указывать:
- Версия библиотеки
- Какой используется МК
- Версия SDK (для ESP)
- Версия Arduino IDE
- Корректно ли работают ли встроенные примеры, в которых используются функции и конструкции, приводящие к багу в вашем коде
- Какой код загружался, какая работа от него ожидалась и как он работает в реальности
- В идеале приложить минимальный код, в котором наблюдается баг. Не полотно из тысячи строк, а минимальный код