[![latest](https://img.shields.io/github/v/release/GyverLibs/GyverHTTP.svg?color=brightgreen)](https://github.com/GyverLibs/GyverHTTP/releases/latest/download/GyverHTTP.zip)
[![PIO](https://badges.registry.platformio.org/packages/gyverlibs/library/GyverHTTP.svg)](https://registry.platformio.org/libraries/gyverlibs/GyverHTTP)
[![Foo](https://img.shields.io/badge/Website-AlexGyver.ru-blue.svg?style=flat-square)](https://alexgyver.ru/)
[![Foo](https://img.shields.io/badge/%E2%82%BD%24%E2%82%AC%20%D0%9F%D0%BE%D0%B4%D0%B4%D0%B5%D1%80%D0%B6%D0%B0%D1%82%D1%8C-%D0%B0%D0%B2%D1%82%D0%BE%D1%80%D0%B0-orange.svg?style=flat-square)](https://alexgyver.ru/support_alex/)
[![Foo](https://img.shields.io/badge/README-ENGLISH-blueviolet.svg?style=flat-square)](https://github-com.translate.goog/GyverLibs/GyverHTTP?_x_tr_sl=ru&_x_tr_tl=en)  

[![Foo](https://img.shields.io/badge/ПОДПИСАТЬСЯ-НА%20ОБНОВЛЕНИЯ-brightgreen.svg?style=social&logo=telegram&color=blue)](https://t.me/GyverLibs)

# GyverHTTP
Очень простой и лёгкий HTTP сервер и полуасинхронный HTTP клиент
- Быстрая отправка и получение файлов
- Удобный минималистичный API

### Совместимость
Совместима со всеми Arduino платформами (используются Arduino-функции)

## Содержание
- [Использование](#usage)
- [Версии](#versions)
- [Установка](#install)
- [Баги и обратная связь](#feedback)

<a id="usage"></a>

## Использование
### StreamWriter
Быстрый отправлятель данных в Print, поддерживает работу с файлами и PROGMEM. Читает в буфер и отправляет блоками, что многократно быстрее обычной отправки
```cpp
StreamWriter(Stream* stream, size_t size);
StreamWriter(const uint8_t* buf, size_t len, bool pgm = 0);

// размер данных
size_t length();

// установить размер блока отправки
void setBlockSize(size_t bsize);

// напечатать в принт
size_t printTo(Print& p);
```

### StreamReader
Быстрый читатель данных из Stream известной длины. Буферизирует и записывает блоками в потребителя, что многократно быстрее обычного чтения
```cpp
StreamReader(Stream* stream = nullptr, size_t len = 0);

// прочитать в строку
String readString();

// установить таймаут
void setTimeout(size_t tout);

// установить размер блока
void setBlockSize(size_t bsize);

// прочитать в буфер, вернёт true при успехе
bool readBytes(uint8_t* buf);

// вывести в write(uint8_t*, size_t)
template <typename T>
bool writeTo(T& p);

size_t printTo(Print& p);

// общий размер входящих данных
size_t length();

// корреткность ридера
operator bool();

Stream* stream;
```

### Client
```cpp
size_t write(uint8_t data);
size_t write(const uint8_t* buffer, size_t size);

// ==========================

// установить новый хост и порт
void setHost(const char* host, uint16_t port);

// установить новый хост и порт
void setHost(const IPAddress& ip, uint16_t port);

// установить новый клиент для связи
void setClient(::Client& client);

// установить таймаут ответа сервера, умолч. 2000 мс
void setTimeout(uint16_t tout);

// обработчик ответов, требует вызова tick() в loop()
void onResponse(ResponseCallback cb);

// ==========================

// подключиться
bool connect();

// отправить запрос
bool request(Text path, Text method, Text headers, FormData& data);
bool request(Text path, Text method, Text headers, Text payload);
bool request(Text path, Text method = "GET", Text headers = Text(), const uint8_t* payload = nullptr, size_t length = 0);

// начать отправку. Дальше нужно вручную print
bool beginSend();

// клиент ждёт ответа
bool isWaiting();

// есть ответ от сервера (асинхронно)
bool available();

// дождаться и прочитать ответ сервера (по available если long poll)
Response getResponse(HeadersCollector* collector = nullptr);

// тикер, вызывать в loop для работы с коллбэком
void tick();

// остановить клиента
void stop();

// пропустить ответ, снять флаг ожидания, остановить если connection close
void flush();
```

### Client::Response
```cpp
// тип контента (из хэдера Content-Type)
Text type();

// код ответа
uint16_t code();

// тело ответа (длина из хэдера Content-Length)
StreamReader& body();

// ответ существует
operator bool();
```

### Client::FormData
// билдер form data
```cpp
void add(Text name, Text filename, Text type, Text data);
```

### Client::Headers
// билдер заголовков
```cpp
void add(Text name, Text value);
```

### Server
```cpp
Server(uint16_t port);

// запустить
void begin();

// вызывать в loop
void tick(HeadersCollector* collector = nullptr);

// подключить обработчик запроса
void onRequest(RequestCallback callback);

// начать ответ. В Headers можно указать кастомные хэдеры
void beginResponse(Headers& resp);

// начать ответ
void beginResponse(uint16_t code = 200);

// доступ к клиенту для отправки
Client* client();

// отправить клиенту код. Должно быть единственным ответом
void send(uint16_t code);

// отправить клиенту и завершить сеанс. Должно быть единственным ответом, использовать без beginResponse
void sendSingle(const Text& text, uint16_t code = 200, Text type = Text());

// отправить клиенту. Можно вызывать несколько раз подряд
void print(Printable& p);
void send(Text text);
void send(Text text, uint16_t code, Text type = Text());

// отправить файл
void sendFile(File& file, Text type = Text(), bool cache = false, bool gzip = false);

// отправить файл-строку как текст
void sendFile(const Text& text, Text type = Text(), bool cache = false);

// отправить файл из буфера
void sendFile(const uint8_t* buf, size_t len, Text type = Text(), bool cache = false, bool gzip = false);

// отправить файл из PROGMEM
void sendFile_P(const uint8_t* buf, size_t len, Text type = Text(), bool cache = false, bool gzip = false);

// отправить файл-строку из PROGMEM
void sendFile_P(const char* pstr, Text type = Text(), bool cache = false);

// пометить запрос как выполненный
void handle();

// использовать CORS хэдеры (умолч. включено)
void useCors(bool use);

// получить mime тип файла по его пути
const __FlashStringHelper* getMime(Text path);
```

### ServerBase::Request
```cpp
// метод запроса
Text method();

// полный урл
Text url();

// путь (без параметров)
Text path();

// получить значение параметра по ключу
// параметр без значения вернёт валидную пустую строку
Text param(Text key);

// получить тело запроса. Может выводиться в Print
StreamReader& body();
```

### ServerBase::Headers
```cpp
// начать с кодом ответа
Headers(uint16_t code);

// добавить хэдер
void add(Text name, Text value);
```

### ghttp::HeadersCollector
Интерфейс для ручной обработки headers. Используется следующим образом:

Создаём свой класс на его основе. Например пусть выводит в сериал
```cpp
class Collector : public ghttp::HeadersCollector {
   public:
    void header(Text& name, Text& value) {
        Serial.print(name);
        Serial.print(": ");
        Serial.println(value);
    }
};
```

Перехват хэдеров в случае с HTTP клиентом
```cpp
if (http.available()) {
    Collector collector;
    ghttp::Client::Response resp = http.getResponse(&collector);
    // в этот момент имеем разобранные хэдеры
    if (resp) ...
}
```

Перехват хэдеров в случае с HTTP сервером
```cpp
Collector collector;

void onrequest(...) {
}

void loop() {
    // хэдеры обработаются перед вызовом коллбэка
    server.tick(&collector);
}
```

<a id="versions"></a>

## Версии
- v1.0
- 1.0.8 - улучшения и добавления

<a id="install"></a>
## Установка
- Библиотеку можно найти по названию **GyverHTTP** и установить через менеджер библиотек в:
    - Arduino IDE
    - Arduino IDE v2
    - PlatformIO
- [Скачать библиотеку](https://github.com/GyverLibs/GyverHTTP/archive/refs/heads/main.zip) .zip архивом для ручной установки:
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