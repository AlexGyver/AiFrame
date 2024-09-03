#pragma once
#include <Arduino.h>
#include <limits.h>

#include "convert/b64.h"
#include "convert/convert.h"
#include "convert/unicode.h"
#include "convert/url.h"
#include "hash.h"

namespace su {

class Text : public Printable {
   public:
    class Cstr {
       public:
        Cstr(Cstr& val) {
            move(val);
        }
        Cstr& operator=(Cstr& val) {
            move(val);
            return *this;
        }

#if __cplusplus >= 201103L
        Cstr(Cstr&& rval) noexcept {
            move(rval);
        }
        Cstr& operator=(Cstr&& rval) noexcept {
            move(rval);
            return *this;
        }
#endif

        Cstr(const Text& t, bool forceDup = false) {
            if (!t.length()) return;
            if (!t.pgm() && t.terminated() && !forceDup) {
                str = t.str();
                len = t.length();
                return;
            }
            char* tstr = new char[t.length() + 1];
            if (!tstr) return;
            t.toStr(tstr);
            str = tstr;
            len = t.length();
            del = true;
        }

        ~Cstr() {
            reset();
        }

        operator const char*() const {
            return str;
        }

        operator bool() const {
            return len;
        }

        void reset() {
            if (del) delete[] str;
            str = "";
            len = 0;
            del = false;
        }

        uint16_t length() {
            return len;
        }

       private:
        const char* str = "";
        uint16_t len = 0;
        bool del = false;

        void move(Cstr& val) noexcept {
            if (this == &val) return;
            reset();
            str = val.str;
            len = val.len;
            del = val.del;
            val.str = "";
            val.del = 0;
            val.len = 0;
        }
    };

    enum class Type : uint8_t {
        constChar,  // const char*
        pgmChar,    // PROGMEM
        value,      // буфер Value
    };
    struct find_t {
        int16_t count = 0;
        int16_t start = 0;
        int16_t end = 0;
        bool last = 0;
    };

    // ========================== CONSTRUCTOR ==========================
    Text() {}
    Text(const __FlashStringHelper* str, int16_t len = -1) : _str((PGM_P)str), _len(len >= 0 ? len : strlen_P((PGM_P)str)), _type(Type::pgmChar) {}
    Text(const char* str, int16_t len = -1, bool pgm = 0) : _str(str), _len(len >= 0 ? len : (pgm ? strlen_P(str) : strlen(str ? str : ""))), _type(pgm ? Type::pgmChar : Type::constChar) {}
    Text(const uint8_t* str, uint16_t len) : _str((const char*)str), _len(len) {}
    Text(const String& str) : _str(str.c_str()), _len(str.length()) {}

    // ========================== SYSTEM ==========================
    // Строка из Flash памяти
    inline bool pgm() const {
        return _type == Type::pgmChar;
    }

    // Длина строки
    uint16_t length() const {
        return valid() ? _len : 0;
    }

    // Длина строки с учётом unicode символов
    uint16_t lengthUnicode() const {
        if (!length()) return 0;
        uint16_t count = 0;
        for (uint16_t i = 0; i < _len; i++) {
            if ((_charAt(i) & 0xc0) != 0x80) count++;
        }
        return count;
    }

    // получить реальную позицию символа в строке, если она содержит юникод
    uint16_t unicodeToPos(uint16_t upos) const {
        if (!length() || upos > length()) return 0;

        for (uint16_t i = 0; i < _len; i++) {
            if ((_charAt(i) & 0xc0) != 0x80) {
                if (!upos) return i;
                else upos--;
            }
        }
        return 0;
    }

    // получить позицию юникод символа в строке, если она содержит юникод
    uint16_t posToUnicode(uint16_t pos) const {
        if (!length() || pos > length()) return 0;

        uint16_t u = 0;
        for (uint16_t i = 0; i < pos; i++) {
            if ((_charAt(i) & 0xc0) != 0x80) u++;
        }
        return u;
    }

    // посчитать и вернуть длину строки (const)
    uint16_t readLen() const {
        return valid() ? (pgm() ? strlen_P(_str) : strlen(_str)) : 0;
    }

    // пересчитать и запомнить длину строки (non-const)
    void calcLen() {
        _len = readLen();
    }

    // Тип строки
    Type type() const {
        return _type;
    }

    // Получить указатель на строку. Всегда вернёт указатель, отличный от nullptr!
    const char* str() const {
        return valid() ? _str : "";
    }

    // указатель на конец строки
    const char* end() const {
        return valid() ? (_str + _len) : "";
    }

    // Статус строки
    inline bool valid() const {
        return _str;
    }

    explicit inline operator bool() const {
        return _str;
    }

    // строка валидна и оканчивается \0
    bool terminated() const {
        return (valid()) ? (_charAt(_len) == 0) : 0;
    }

    // Напечатать в Print
    virtual size_t printTo(Print& p) const {
        if (!length()) return 0;
        size_t ret = 0;
        if (pgm()) {
            if (terminated()) {
                ret = p.print((const __FlashStringHelper*)_str);
            } else {
                for (uint16_t i = 0; i < _len; i++) ret += p.write(_charAt(i));
            }
        } else {
            ret = p.write(_str, _len);
        }
        return ret;
    }

    // ========================== COMPARE ==========================

    bool operator==(const Text& s) const {
        return compare(s);
    }
    bool operator!=(const Text& s) const {
        return !compare(s);
    }
    bool operator==(const char* s) const {
        return compare(s);
    }
    bool operator!=(const char* s) const {
        return !compare(s);
    }
    bool operator==(const __FlashStringHelper* s) const {
        return compare(s);
    }
    bool operator!=(const __FlashStringHelper* s) const {
        return !compare(s);
    }
    bool operator==(const String& s) const {
        return compare(s);
    }
    bool operator!=(const String& s) const {
        return !compare(s);
    }

    /**
       @brief Сравнить со строкой

       @param s
       @return true строки совпадают
       @return false строки не совпадают
    */
    bool compare(const char* s) const {
        return (length() && s) ? !_compare(_str, s, false, _len) : 0;
    }
    bool compare(const __FlashStringHelper* s) const {
        return (length() && s) ? !_compare(_str, (PGM_P)s, true, _len) : 0;
    }
    bool compare(const Text& txt) const {
        return (txt.length() == _len) ? !_compareN(_str, txt._str, txt.pgm(), _len) : 0;
    }

    /**
       @brief Сравнить со строкой с указанием количества символов

       @param s строка любого типа
       @param amount количество символов
       @param from индекс, откуда начинать сравнение
       @return true строки совпадают
       @return false строки не совпадают
    */
    bool compareN(const Text& txt, uint16_t amount, uint16_t from = 0) const {
        if (!valid() || !txt.valid() || !amount || amount > txt._len || from + amount > _len) return 0;
        return !_compareN(_str + from, txt._str, txt.pgm(), amount);
    }
    bool compareN(const char* s, uint16_t amount, uint16_t from = 0) const {
        return (valid() && from + amount <= _len) ? !_compareN(_str + from, s, false, amount) : 0;
    }
    bool compareN(const __FlashStringHelper* s, uint16_t amount, uint16_t from = 0) const {
        return (valid() && from + amount <= _len) ? !_compareN(_str + from, (PGM_P)s, true, amount) : 0;
    }

    // ========================== SEARCH ==========================

    // найти символ и получить указатель на первое вхождение
    const char* find(char sym, uint16_t from = 0) const {
        if (!length()) return nullptr;
        int16_t idx = indexOf(sym, from);
        return (idx < 0) ? nullptr : (_str + idx);
    }

    // начинается с
    bool startsWith(char c) const {
        return length() ? (_charAt(0) == c) : 0;
    }

    // начинается со строки
    bool startsWith(const char* s) const {
        return length() ? !_compareEnd(_str, s, false, _len) : 0;
    }
    bool startsWith(const __FlashStringHelper* s) const {
        return length() ? !_compareEnd(_str, (PGM_P)s, true, _len) : 0;
    }
    bool startsWith(const Text& txt) const {
        return (length() && txt.length() && txt._len <= _len) ? !_compareN(_str, txt._str, txt.pgm(), txt._len) : 0;
    }

    // заканчивается строкой
    bool endsWith(const Text& txt) const {
        return (length() && txt.length() && txt._len <= _len) ? !_compareN(_str + _len - txt._len, txt._str, txt.pgm(), txt._len) : 0;
    }
    bool endsWith(char c) {
        return length() ? (_charAt(_len - 1) == c) : 0;
    }

    // Найти позицию символа в строке
    int16_t indexOf(char sym, uint16_t from = 0) const {
        if (!length() || from > _len) return -1;
        for (uint16_t i = from; i < _len; i++) {
            if (_charAt(i) == sym) return i;
        }
        return -1;
    }

    // Найти позицию строки в строке
    int16_t indexOf(const Text& txt, uint16_t from = 0) const {
        if (!length() || !txt.length() || (from + txt._len) > _len) return -1;
        for (uint16_t i = from; i <= _len - txt._len; i++) {
            if (!_compareN(_str + i, txt._str, txt.pgm(), txt._len)) return i;
        }
        return -1;
    }
    int16_t indexOf(const char* s, uint16_t from = 0) const {
        if (!length()) return -1;
        for (uint16_t i = from; i < _len; i++) {
            if (!_compareEnd(_str + i, s, false, _len - i)) return i;
        }
        return -1;
    }
    int16_t indexOf(const __FlashStringHelper* s, uint16_t from = 0) const {
        if (!length()) return -1;
        for (uint16_t i = from; i < _len; i++) {
            if (!_compareEnd(_str + i, (PGM_P)s, true, _len - i)) return i;
        }
        return -1;
    }

    // Найти позицию строки в строке, результат в юникод-позиции
    int16_t indexOfUnicode(const Text& txt, uint16_t from = 0) const {
        int16_t pos = indexOf(txt, from);
        return pos > 0 ? posToUnicode(pos) : pos;
    }
    int16_t indexOfUnicode(const char* s, uint16_t from = 0) const {
        int16_t pos = indexOf(s, from);
        return pos > 0 ? posToUnicode(pos) : pos;
    }
    int16_t indexOfUnicode(const __FlashStringHelper* s, uint16_t from = 0) const {
        int16_t pos = indexOf(s, from);
        return pos > 0 ? posToUnicode(pos) : pos;
    }

    /**
     @brief Найти позицию символа в строке с конца

    @param sym символ
    @return int16_t позиция символа, -1 если не найден
    */
    int16_t lastIndexOf(char sym) const {
        if (!length()) return -1;
        for (int16_t i = _len - 1; i >= 0; i--) {
            if (_charAt(i) == sym) return i;
        }
        return -1;
    }

    /**
      @brief Найти позицию строки в строке с конца

      @param str строка
      @return int16_t позиция строки, -1 если не найдена
    */
    int16_t lastIndexOf(const Text& txt) const {
        if (!length() || !txt.length() || txt._len > _len) return -1;
        for (int16_t i = _len - txt._len; i >= 0; i--) {
            if (!_compareN(_str + i, txt._str, txt.pgm(), txt._len)) return i;
        }
        return -1;
    }

    // Найти позицию строки в строке с конца, результат в юникод-позиции
    int16_t lastIndexOfUnicode(const Text& txt) const {
        int16_t pos = lastIndexOf(txt);
        return pos > 0 ? posToUnicode(pos) : pos;
    }

    // ========================== SUB ==========================

    // Посчитать количество подстрок, разделённых символом (количество разделителей +1)
    uint16_t count(char sym) const {
        if (!length()) return 0;
        uint16_t sum = 1;
        for (uint16_t i = 0; i < _len; i++) {
            if (_charAt(i) == sym) sum++;
        }
        return sum;
    }

    // Посчитать количество подстрок, разделённых строками (количество разделителей +1)
    uint16_t count(const Text& txt) const {
        if (!length() || !txt.length()) return 0;
        uint16_t sum = 1;
        int16_t pos = 0;
        while (1) {
            pos = indexOf(txt, pos);
            if (pos < 0) break;
            pos += txt._len;
            sum++;
        }
        return sum;
    }

    /**
       @brief Получить подстроку из списка по индексу

       @param idx индекс
       @param div символ-разделитель
       @return Text подстрока
    */
    Text getSub(uint16_t idx, char div) const {
        if (!length()) return Text();
        int16_t start = 0, end = 0;
        while (1) {
            end = indexOf(div, end);
            if (end < 0) end = _len;
            if (!idx--) return Text(_str + start, end - start, pgm());
            if ((uint16_t)end == _len) break;
            end++;
            start = end;
        }
        return Text();
    }

    /**
      @brief Получить подстроку из списка по индексу

      @param idx индекс
      @param div строка-разделитель
      @return Text подстрока
    */
    Text getSub(uint16_t idx, const Text& div) const {
        if (!length() || !div.length() || div._len > _len) return Text();
        int16_t start = 0, end = 0;
        while (1) {
            end = indexOf(div, end);
            if (end < 0) end = _len;
            if (!idx--) return Text(_str + start, end - start, pgm());
            if ((uint16_t)end == _len) break;
            end += div._len;
            start = end;
        }
        return Text();
    }

    // ========================== SPLIT ==========================

    /**
       @brief Разделить по символу-разделителю

       @param arr внешний массив любого типа (Text, числа)
       @param len размер массива
       @param div символ разделитель
       @return uint16_t количество найденных подстрок
    */
    template <typename T>
    uint16_t split(T* arr, uint16_t len, char div) const {
        if (!len || !length()) return 0;
        find_t f;

        while (!f.last) {
            Text txt = _parse(div, 1, len, f);
            arr[f.count - 1] = txt;
        }
        return f.count;
    }

    template <typename T>
    uint16_t split(T** arr, uint16_t len, char div) const {
        if (!len || !length()) return 0;
        find_t f;
        while (!f.last) {
            Text txt = _parse(div, 1, len, f);
            *(arr[f.count - 1]) = txt;
        }
        return f.count;
    }

    /**
      @brief Разделить по строке-разделителю

      @param arr внешний массив любого типа (Text, числа)
      @param len размер массива
      @param div строка разделитель
      @return uint16_t количество найденных подстрок
    */
    template <typename T>
    uint16_t split(T* arr, uint16_t len, const Text& div) const {
        if (!len || !length() || !div.length() || div._len > _len) return 0;
        find_t f;
        while (!f.last) {
            Text txt = _parse(div, div._len, len, f);
            arr[f.count - 1] = txt;
        }
        return f.count;
    }

    template <typename T>
    uint16_t split(T** arr, uint16_t len, const Text& div) const {
        if (!len || !length() || !div.length() || div._len > _len) return 0;
        find_t f;
        while (!f.last) {
            Text txt = _parse(div, div._len, len, f);
            *(arr[f.count - 1]) = txt;
        }
        return f.count;
    }

    // ========================== EXPORT ==========================

    // вернёт новую строку с убранными пробельными символами с начала и конца
    Text trim() const {
        if (!length()) return Text();
        Text txt(*this);
        while (txt._len) {
            uint8_t sym = txt._charAt(0);
            if (sym && (sym <= 0x0F || sym == ' ')) {
                txt._str++;
                txt._len--;
            } else break;
        }
        while (txt._len) {
            uint8_t sym = txt._charAt(txt._len - 1);
            if (sym <= 0x0F || sym == ' ') txt._len--;
            else break;
        }
        return txt;
    }

    // выделить подстроку (начало, конец не включая). Отрицательные индексы работают с конца строки
    Text substring(int16_t start, int16_t end = 0) const {
        if (!length()) return Text();
        if (start < 0) start += _len;
        if (!end) end = _len;
        else if (end < 0) end += _len;
        if (start > (int16_t)_len || end > (int16_t)_len) return Text();

        if (end && end < start) {
            int16_t b = end;
            end = start;
            start = b;
        }
        return Text(_str + start, end - start, pgm());
    }

    // выделить подстроку с содержанием юникода (начало, конец не включая). Отрицательные индексы работают с конца строки
    Text substringUnicode(int16_t start, int16_t end = 0) const {
        if (start < 0 || end < 0) {
            uint16_t ulen = lengthUnicode();
            if (start < 0) start += ulen;
            if (end < 0) end += ulen;
        }
        if (start < 0 || end < 0 || start > (int16_t)_len || end > (int16_t)_len) return Text();
        return substring(unicodeToPos(start), end ? unicodeToPos(end) : 0);
    }

    // Добавить к String строке. Вернёт false при неудаче
    virtual bool addString(String& s) const {
        if (!length() || !_len) return 0;
        if (!s.reserve(s.length() + _len)) return 0;
        if (pgm()) {
            if (terminated()) {
                s += (const __FlashStringHelper*)_str;
            } else {
                for (uint16_t i = 0; i < _len; i++) s += _charAt(i);
            }
        } else {
#if defined(ESP8266) || defined(ESP32)
            s.concat(_str, _len);
#else
            if (terminated()) {
                s.concat(_str);
            } else {
                const char* p = _str;
                for (uint16_t i = 0; i < _len; i++) s += p[i];
            }
#endif
        }
        return 1;
    }

    // Добавить к String строке. Вернёт false при неудаче
    bool addString(String& s, bool decodeUnicode) const {
        if (!length() || pgm()) return 0;
        if (decodeUnicode) {
            s += unicode::decode(_str, _len);
        } else {
            addString(s);
        }
        return 1;
    }

    // получить как строку, раскодировать unicode
    String decodeUnicode() const {
        if (!length() || pgm()) return String();
        return unicode::decode(_str, _len);
    }

    // получить как строку, раскодировать urlencode
    String decodeUrl() const {
        if (!length() || pgm()) return String();
        return url::decode(_str, _len);
    }

    // Получить символ по индексу. Допускаются отрицательные
    char charAt(int idx) const {
        if (idx < 0) idx += length();
        if (idx < 0) return 0;
        return (valid() && idx < _len) ? _charAt(idx) : 0;
    }

    // Получить символ по индексу. Допускаются отрицательные
    char operator[](int idx) const {
        return charAt(idx);
    }

    // ========================== B64 ==========================

    // размер данных (байт), если они b64
    size_t sizeB64() const {
        if (!length()) return 0;
        return b64::decodedLen(_str, _len);
    }

    // вывести в переменную из b64
    bool decodeB64(void* var, size_t size) const {
        if (sizeB64() == size) {
            b64::decode((uint8_t*)var, _str, _len);
            return 1;
        }
        return 0;
    }

    // ========================== HASH ==========================

    // хэш строки, размер зависит от платформы (size_t)
    size_t hash() const {
        return valid() ? (pgm() ? su::hash_P(_str, _len) : su::hash(_str, _len)) : 0;
    }

    // хэш строки 32 бит
    uint32_t hash32() const {
        return valid() ? (pgm() ? su::hash32_P(_str, _len) : su::hash32(_str, _len)) : 0;
    }

    // ========================== CONVERT ==========================

    // получить const char* копию (Cstr конвертируется в const char*). Всегда валидна и терминирована. Если Text из PGM или не терминирован - будет создана временная копия
    Cstr c_str(bool forceDup = false) const {
        return Cstr(*this, forceDup);
    }

    // Вывести в String строку. Вернёт false при неудаче
    bool toString(String& s) const {
        s = "";
        return addString(s);
    }

    // Вывести в String строку. Вернёт false при неудаче
    bool toString(String& s, bool decodeUnicode) const {
        s = "";
        return addString(s, decodeUnicode);
    }

    // Получить как String строку
    String toString() const {
        String s;
        addString(s);
        return s;
    }

    // Получить как String строку
    String toString(bool decodeUnicode) const {
        String s;
        addString(s, decodeUnicode);
        return s;
    }

    // Вывести в char массив. Вернёт длину строки. terminate - завершить строку нулём
    uint16_t toStr(char* buf, int16_t bufsize = -1, bool terminate = true) const {
        if (!bufsize) return 0;
        if (!length()) {
            if (terminate) buf[0] = 0;
            return 0;
        }
        if (bufsize > 0 && (int16_t)(_len + terminate) > bufsize) return 0;
        pgm() ? strncpy_P(buf, _str, _len) : strncpy(buf, _str, _len);
        if (terminate) buf[_len] = 0;
        return _len;
    }

    // получить значение как bool
    bool toBool() const {
        return valid() && (charAt(0) == 't' || charAt(0) == '1');
    }

    // получить значение как int
    int toInt() const {
#if (UINT_MAX == UINT32_MAX)
        return toInt32();
#else
        return toInt16();
#endif
    }

    // получить значение как int 16
    int16_t toInt16() const {
        if (!valid()) return 0;
        return pgm() ? strToInt_P<int16_t>(_str, _len) : strToInt<int16_t>(_str, _len);
    }

    // получить значение как int 32
    int32_t toInt32() const {
        if (!valid()) return 0;
        if (_len < 5) return toInt16();
        return pgm() ? strToInt_P<int32_t>(_str, _len) : strToInt<int32_t>(_str, _len);
    }

    // получить значение как uint 32 из HEX строки
    uint32_t toInt32HEX() const {
        if (!length()) return 0;
        uint8_t i = 0;
        uint32_t v = 0;
        if (_len > 2 && _charAt(0) == '0' && _charAt(1) == 'x') i += 2;
        for (; i < _len; i++) {
            char sym = _charAt(i);
            switch (sym) {
                case '0' ... '9':
                case 'a' ... 'f':
                case 'A' ... 'F':
                    break;
                default:
                    return v;
            }
            v <<= 4;
            v += (sym & 0xf) + (sym > '9' ? 9 : 0);
        }
        return v;
    }

    // получить значение как int64
    int64_t toInt64() const {
        if (!valid()) return 0;
        if (_len < 10) return toInt32();
        return pgm() ? strToInt_P<int64_t>(_str, _len) : strToInt<int64_t>(_str, _len);
    }

    // получить значение как float
    float toFloat() const {
        if (!valid()) return 0;
        if (pgm()) {
            char buf[_len + 1];
            strncpy_P(buf, _str, _len);
            buf[_len] = 0;
            return atof(buf);
        } else {
            return atof(_str);
        }
    }

// ================= CAST =================
#define T_MAKE_OPERATOR_EXPL(type, func)  \
    explicit operator type() const {      \
        return (type)func();              \
    }                                     \
    bool operator==(const type v) const { \
        return (type)func() == v;         \
    }                                     \
    bool operator!=(const type v) const { \
        return (type)func() != v;         \
    }                                     \
    bool operator>(const type v) const {  \
        return (type)func() > v;          \
    }                                     \
    bool operator<(const type v) const {  \
        return (type)func() < v;          \
    }                                     \
    bool operator>=(const type v) const { \
        return (type)func() >= v;         \
    }                                     \
    bool operator<=(const type v) const { \
        return (type)func() <= v;         \
    }

#define T_MAKE_OPERATOR(type, func)       \
    operator type() const {               \
        return (type)func();              \
    }                                     \
    bool operator==(const type v) const { \
        return (type)func() == v;         \
    }                                     \
    bool operator!=(const type v) const { \
        return (type)func() != v;         \
    }                                     \
    bool operator>(const type v) const {  \
        return (type)func() > v;          \
    }                                     \
    bool operator<(const type v) const {  \
        return (type)func() < v;          \
    }                                     \
    bool operator>=(const type v) const { \
        return (type)func() >= v;         \
    }                                     \
    bool operator<=(const type v) const { \
        return (type)func() <= v;         \
    }

    // T_MAKE_OPERATOR(bool, toBool)
    T_MAKE_OPERATOR_EXPL(char, toInt16)
    T_MAKE_OPERATOR(signed char, toInt16)
    T_MAKE_OPERATOR(unsigned char, toInt16)
    T_MAKE_OPERATOR(short, toInt16)
    T_MAKE_OPERATOR(unsigned short, toInt16)
    T_MAKE_OPERATOR(int, toInt)
    T_MAKE_OPERATOR(unsigned int, toInt)
    T_MAKE_OPERATOR(long, toInt32)
    T_MAKE_OPERATOR(unsigned long, toInt32)
    T_MAKE_OPERATOR(long long, toInt64)
    T_MAKE_OPERATOR(unsigned long long, toInt64)
    T_MAKE_OPERATOR(float, toFloat)
    T_MAKE_OPERATOR(double, toFloat)

    operator String() const {
        return toString();
    }
    // operator const char*() const {
    //     return c_str();
    // }

    const char* _str = nullptr;
    uint16_t _len = 0;
    Type _type = Type::constChar;

    // получить символ по индексу (без проверок валидности)
    char _charAt(uint16_t idx) const {
        return pgm() ? (char)pgm_read_byte(_str + idx) : *(_str + idx);
    }

    template <typename T>
    Text _parse(const T& div, uint16_t divlen, int16_t len, find_t& f) const {
        if (f.count) f.start = f.end = f.end + divlen;
        f.end = indexOf(div, f.end);
        if (f.end < 0 || f.count + 1 == len) f.end = _len;
        f.count++;
        if (f.count == len || f.end == (int16_t)_len) f.last = 1;
        return Text(_str + f.start, f.end - f.start, pgm());
    }

   protected:
    uint16_t _compareN(const char* s1, const char* s2, bool pgm2, uint16_t len) const {
        while (len) {
            if ((pgm() ? pgm_read_byte(s1++) : *s1++) != (pgm2 ? pgm_read_byte(s2++) : *s2++)) return len;
            len--;
        }
        return 0;
    }
    uint16_t _compare(const char* s1, const char* s2, bool pgm2, uint16_t len) const {
        uint16_t left = _compareN(s1, s2, pgm2, len);
        return left ? left : (pgm2 ? pgm_read_byte(s2 + len) : *(s2 + len));
    }
    bool _compareEnd(const char* s1, const char* s2, bool pgm2, uint16_t len) const {
        uint16_t left = _compareN(s1, s2, pgm2, len);
        return pgm2 ? pgm_read_byte(s2 + len - left) : *(s2 + len - left);
    }
};

}  // namespace su