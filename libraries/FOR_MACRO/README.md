[![latest](https://img.shields.io/github/v/release/GyverLibs/FOR_MACRO.svg?color=brightgreen)](https://github.com/GyverLibs/FOR_MACRO/releases/latest/download/FOR_MACRO.zip)
[![PIO](https://badges.registry.platformio.org/packages/gyverlibs/library/FOR_MACRO.svg)](https://registry.platformio.org/libraries/gyverlibs/FOR_MACRO)
[![Foo](https://img.shields.io/badge/Website-AlexGyver.ru-blue.svg?style=flat-square)](https://alexgyver.ru/)
[![Foo](https://img.shields.io/badge/%E2%82%BD%24%E2%82%AC%20%D0%9F%D0%BE%D0%B4%D0%B4%D0%B5%D1%80%D0%B6%D0%B0%D1%82%D1%8C-%D0%B0%D0%B2%D1%82%D0%BE%D1%80%D0%B0-orange.svg?style=flat-square)](https://alexgyver.ru/support_alex/)
[![Foo](https://img.shields.io/badge/README-ENGLISH-blueviolet.svg?style=flat-square)](https://github-com.translate.goog/GyverLibs/FOR_MACRO?_x_tr_sl=ru&_x_tr_tl=en)  

[![Foo](https://img.shields.io/badge/ПОДПИСАТЬСЯ-НА%20ОБНОВЛЕНИЯ-brightgreen.svg?style=social&logo=telegram&color=blue)](https://t.me/GyverLibs)

# FOR_MACRO
Variadic for макрос

### Совместимость
Совместима со всеми Arduino платформами (используются Arduino-функции)

## Содержание
- [Использование](#usage)
- [Версии](#versions)
- [Установка](#install)
- [Баги и обратная связь](#feedback)

<a id="usage"></a>

## Использование
`FOR_MACRO` - макрос, позволяющий применить другой макрос к variadic списку аргументов, по сути вызывает указанный макрос для каждого аргумента. По умолчанию поддерживает максимум 512 аргументов, можно сгенерировать макрос на любое количество аргументов при помощи приложенного python-скрипта.

Для использования макроса нужно создать два своих макроса - один будет макро-функцией, которая применяется к каждому аргументу, а вторая - сам макрос, который будет использоваться. Первая макро-функция должна иметь вид `F(N, i, p, val)`, где:
- `N` - количество аргументов
- `i` - счётчик, начинается с конца-1 из за особенностей реализации
- `p` - параметр, может быть чем угодно. Можно поставить `0` если не нужен
- `val` - текущий аргумент

Как это работает будет понятнее на примерах:
```cpp
#define MF1(N, i, p, val) N,
#define FOR_1(...) FOR_MACRO(MF1, 0, __VA_ARGS__)

FOR_1(test, kek, string);   // развернётся в 3, 3, 3,
```
```cpp
#define MF2(N, i, p, val) i,
#define FOR_2(...) FOR_MACRO(MF2, 0, __VA_ARGS__)

FOR_2(test, kek, string);   // развернётся в 2, 1, 0
```
```cpp
#define MF3(N, i, p, val) p,
#define FOR_3(...) FOR_MACRO(MF3, 0, __VA_ARGS__)

FOR_3(test, kek, string);   // развернётся в 0, 0, 0 (параметр)
```
```cpp
#define MF4(N, i, p, val) val,
#define FOR_4(...) FOR_MACRO(MF4, 0, __VA_ARGS__)

FOR_4(test, kek, string);   // развернётся в test, kek, string,
```
```cpp
#define MF5(N, i, p, val) #val,
#define FOR_5(...) FOR_MACRO(MF5, 0, __VA_ARGS__)

FOR_5(test, kek, string);   // развернётся в "test", "kek", "string",
```

Более реальный пример - создание строк
```cpp
#define MF6(N, i, p, val) const char* p##_##i = #val;
#define FOR_6(name, ...) FOR_MACRO(MF6, name, __VA_ARGS__)

FOR_6(strings, test, kek, string);
// развернётся в 
// const char* strings_2 = "test"; 
// const char* strings_1 = "kek"; 
// const char* strings_0 = "string";
```
```cpp
#define MF7(N, i, p, val) const char* val = #val;
#define FOR_7(name, ...) FOR_MACRO(MF7, name, __VA_ARGS__)

FOR_7(strings, test, kek, string);
// развернётся в 
// const char* test = "test"; 
// const char* kek = "kek"; 
// const char* string = "string";
```

<a id="versions"></a>

## Версии
- v1.0

<a id="install"></a>
## Установка
- Библиотеку можно найти по названию **FOR_MACRO** и установить через менеджер библиотек в:
    - Arduino IDE
    - Arduino IDE v2
    - PlatformIO
- [Скачать библиотеку](https://github.com/GyverLibs/FOR_MACRO/archive/refs/heads/main.zip) .zip архивом для ручной установки:
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