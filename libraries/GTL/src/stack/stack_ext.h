#pragma once
#include <inttypes.h>
#include <stddef.h>

namespace gtl {

template <typename T>
struct bsearch_t {
    // индекс в стеке
    int idx;

    // указатель на данные, если найдены
    T* ptr;

    explicit operator bool() {
        return ptr;
    }
};

// External Static Stack
template <typename T>
class stack_ext {
   public:
    stack_ext() {}
    stack_ext(T* arr, size_t capacity, size_t length = 0) {
        setBuffer(arr, capacity, length);
    }

    // подключить буфер
    void setBuffer(T* arr, size_t capacity, size_t length = 0) {
        _buf = arr;
        _cap = capacity;
        _len = min(length, capacity);
    }

    // добавить в конец
    bool push(const T& val) {
        if (!_fit(_len + 1)) return 0;
        _buf[_len++] = val;
        return 1;
    }

    // добавить в конец
    bool operator+=(const T& val) {
        return push(val);
    }

    // получить с конца и удалить
    T& pop() {
        return _buf[(_len--) - 1];
    }

    // прочитать с конца не удаляя
    T& peek() const {
        return _buf[_len - 1];
    }

    // добавить в начало
    bool shift(const T& val) {
        if (!_fit(_len + 1)) return 0;
        memmove((void*)(_buf + 1), (const void*)(_buf), _len * sizeof(T));
        _buf[0] = val;
        _len++;
        return 1;
    }

    // получить с начала и удалить
    T unshift() {
        if (!length()) return T();
        T t = _buf[0];
        _len--;
        memmove((void*)(_buf), (const void*)(_buf + 1), _len * sizeof(T));
        return t;
    }

    // прочитать с начала не удаляя
    T& unpeek() const {
        return _buf[0];
    }

    // бинарный поиск в отсортированном стеке
    bsearch_t<T> searchSort(const T& val) {
        if (!length()) return bsearch_t<T>{0, nullptr};
        int low = 0, high = length() - 1;
        while (low <= high) {
            int mid = low + ((high - low) >> 1);
            if (_buf[mid] == val) return bsearch_t<T>{mid, &_buf[mid]};
            if (_buf[mid] < val) low = mid + 1;
            else high = mid - 1;
        }
        return bsearch_t<T>{low, nullptr};
    }

    // добавить с сортировкой. Флаг uniq - не добавлять если элемент уже есть
    bool addSort(const T& val, bool uniq = false) {
        bsearch_t<T> pos = searchSort(val);
        if (uniq && pos) return 0;
        return insert(pos.idx, val);
    }

    // удалить элемент. Отрицательный - с конца
    bool remove(int idx) {
        if (!length() || idx >= (int)_len || idx < -(int)_len) return 0;

        if (idx < 0) idx += _len;
        memcpy((void*)(_buf + idx), (const void*)(_buf + idx + 1), (_len - idx - 1) * sizeof(T));
        _len--;
        return 1;
    }

    // удалить несколько элементов, начиная с индекса
    bool remove(size_t from, size_t amount) {
        if (!length() || !amount || from >= _len) return 0;
        size_t to = from + amount;
        if (to >= _len - 1) {
            _len = from;
            return 1;
        }

        memmove((void*)(_buf + from), (const void*)(_buf + to), (_len - to) * sizeof(T));
        _len -= amount;
        return 1;
    }

    // вставить элемент на индекс (отрицательный индекс - с конца, слишком большой - будет push)
    bool insert(int idx, const T& val) {
        if (idx < 0) idx += _len;
        if (idx < 0) return 0;
        if (idx == 0) return shift(val);
        else if (idx >= (int)_len) return push(val);

        if (!_fit(_len + 1)) return 0;
        memmove((void*)(_buf + idx + 1), (const void*)(_buf + idx), (_len - idx) * sizeof(T));
        _buf[idx] = val;
        _len++;
        return 1;
    }

    // прибавить другой массив в конец
    bool concat(const stack_ext& st) {
        return concat(st._buf, st._len);
    }

    // прибавить другой массив в конец
    bool concat(const T* buf, size_t len, bool pgm = false) {
        if (!len) return 1;
        if (!buf || !_fit(_len + len)) return 0;

        if (pgm) {
            memcpy_P((void*)(_buf + _len), (const void*)(buf), len * sizeof(T));
        } else {
            memcpy((void*)(_buf + _len), (const void*)(buf), len * sizeof(T));
        }
        _len += len;
        return 1;
    }

    // прибавить другой массив в конец, синоним concat
    size_t write(const T* buf, size_t len) {
        return concat(buf, len, false) ? len : 0;
    }

    // прибавить другой массив в конец
    bool operator+=(const stack_ext& st) {
        return concat(st);
    }

    // заполнить значением (на capacity)
    void fill(const T& val) {
        for (size_t i = 0; i < capacity(); i++) _buf[i] = val;
        _len = _cap;
    }

    // инициализировать, вызвать конструкторы (на capacity)
    void init() {
        fill(T());
    }

    // очистить (установить длину 0)
    void clear() {
        _len = 0;
    }

    // количество элементов
    size_t length() const {
        return _buf ? _len : 0;
    }

    // установить количество элементов (само вызовет reserve)
    bool setLength(size_t len) {
        if (!_fit(_len + len)) return 0;
        _len = len;
        return 1;
    }

    // есть место для добавления
    bool canAdd() const {
        return _buf ? (_len < _cap) : 0;
    }

    // вместимость, элементов
    size_t capacity() const {
        return _buf ? _cap : 0;
    }

    // текущий размер в байтах
    size_t size() const {
        return length() * sizeof(T);
    }

    // позиция элемента (-1 если не найден)
    int indexOf(const T& val) const {
        if (_buf) {
            for (size_t i = 0; i < _len; i++) {
                if (_buf[i] == val) return i;
            }
        }
        return -1;
    }

    // содержит элемент
    bool has(const T& val) const {
        return indexOf(val) != -1;
    }

    // удалить по значению (true если элемента нет)
    bool removeByVal(const T& val) {
        int i = indexOf(val);
        return (i >= 0) ? remove(i) : true;
    }

    // получить элемент под индексом. Отрицательный - с конца
    T& get(int idx) const {
        if (idx < 0) return _buf[(idx + _len >= 0) ? (idx + _len) : 0];
        return _buf[idx];
    }

    // получить элемент под индексом. Отрицательный - с конца
    T& operator[](int idx) const {
        return get(idx);
    }

    // доступ к буферу
    inline T* buf() const {
        return _buf;
    }

    // буфер существует
    inline bool valid() const {
        return _buf;
    }

    // буфер существует
    explicit operator bool() const {
        return _buf;
    }

    //
    bool includes(const T& val) const __attribute__((deprecated)) {
        return has(val);
    }

   protected:
    T* _buf = nullptr;
    size_t _len = 0;
    size_t _cap = 0;

    inline bool _fit(size_t size) {
        return (size <= _cap || reserve(size)) && _buf;
    }
    virtual bool reserve(size_t size) {
        return 0;
    }
};

}  // namespace gtl