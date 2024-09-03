[![latest](https://img.shields.io/github/v/release/GyverLibs/AutoOTA.svg?color=brightgreen)](https://github.com/GyverLibs/AutoOTA/releases/latest/download/AutoOTA.zip)
[![PIO](https://badges.registry.platformio.org/packages/gyverlibs/library/AutoOTA.svg)](https://registry.platformio.org/libraries/gyverlibs/AutoOTA)
[![Foo](https://img.shields.io/badge/Website-AlexGyver.ru-blue.svg?style=flat-square)](https://alexgyver.ru/)
[![Foo](https://img.shields.io/badge/%E2%82%BD%24%E2%82%AC%20%D0%9F%D0%BE%D0%B4%D0%B4%D0%B5%D1%80%D0%B6%D0%B0%D1%82%D1%8C-%D0%B0%D0%B2%D1%82%D0%BE%D1%80%D0%B0-orange.svg?style=flat-square)](https://alexgyver.ru/support_alex/)
[![Foo](https://img.shields.io/badge/README-ENGLISH-blueviolet.svg?style=flat-square)](https://github-com.translate.goog/GyverLibs/AutoOTA?_x_tr_sl=ru&_x_tr_tl=en)  

[![Foo](https://img.shields.io/badge/ПОДПИСАТЬСЯ-НА%20ОБНОВЛЕНИЯ-brightgreen.svg?style=social&logo=telegram&color=blue)](https://t.me/GyverLibs)

# AutoOTA
Библиотека для автоматической проверки ОТА обновлений проекта с GitHub и других источников

### Совместимость
ESP8266/ESP32

## Содержание
- [Использование](#usage)
- [Версии](#versions)
- [Установка](#install)
- [Баги и обратная связь](#feedback)

<a id="usage"></a>

## Использование
- Указать в прошивке текущую версию и путь к файлу с информацией project.json
- При обновлении загрузить бинарники и повысить версию в файле информации
- Программа проверит версию и предложит обновиться

> Библиотека задумана для обновления проектов с GitHub, поэтому настройки по умолчанию для него. Можно обновляться и со своего домена, но хост где хранится файл должен быть тот же, где хранится бинарник, в библиотеке предусмотрена только одна настройка хоста и порта.

```cpp
AutoOTA(const char* cur_ver, const char* path, const char* host = "raw.githubusercontent.com", uint16_t port = 443);

// текущая версия
const char* version();

// проверить обновления. Можно передать строки для записи информации
bool checkUpdate(String* version = nullptr, String* notes = nullptr);

// есть обновление. Вызывать после проверки. Само сбросится в false
bool hasUpdate();

// обновить прошивку из loop
void update();

// обновить прошивку сейчас и перезагрузить чип
bool updateNow();

// тикер, вызывать в loop. Вернёт true при попытке обновления
bool tick();

// есть ошибка
bool hasError();

// прочитать ошибку
Error getError();
```

### Примеры
Локальный объект, обновление сразу
```cpp
AutoOTA ota("1.0", "GyverLibs/GyverHub-example/main/project.json");
if (ota.checkUpdate()) {
    ota.updateNow();
}
```

Обновление из loop
```cpp
AutoOTA ota("1.0", "GyverLibs/GyverHub-example/main/project.json");

void setup() {
    if (ota.checkUpdate()) {
        ota.update();
    }
}

void loop() {
    ota.tick();
}
```

Получение инфо об обновлении
```cpp
String ver, notes;
if (ota.checkUpdate(&ver, &notes)) {
    Serial.println(ver);
    Serial.println(notes);
}
```

## Файл project.json
Файл содержит информацию о проекте и пути к файлам скомпилированной прошивки для разных платформ в формате, который используется в GyverHub и ESPHome:

```json
{
  "name": "Название проекта",
  "about": "Краткое описание проекта",
  "version": "1.0",
  "notes": "Комментарии к обновлению",
  "builds": [
    {
      "chipFamily": "ESP8266",
      "parts": [
        {
          "path": "https://raw.githubusercontent.com/GyverLibs/GyverHub-example/main/bin/firmware.bin",
          "offset": 0
        }
      ]
    }
  ]
}
```

### Массив builds
Если проект может быть запущен на разных ESP-шках - можно приложить отдельный бинарник для каждой и указать пути к ним. Библиотека сама определяет, на какой платформе запущена и выберет нужный файл. Полный пример со всем семейством ESP можно посмотреть [тут](https://github.com/GyverLibs/AutoOTA/blob/main/project.json). 

### Параметр chipFamily
Поддерживаемые платформы и значения параметра `chipFamily`:
- `ESP8266`
- `ESP32`
- `ESP32-C3`
- `ESP32-C6`
- `ESP32-S2`
- `ESP32-S3`
- `ESP32-H2`

### Путь path
Путь должен вести к скомпилированному файлу прошивки. Его можно разместить как в самом репозитории, так и в релизах:

#### В репозитории
```
https://raw.githubusercontent.com/<аккаунт>/<проект>/main/<путь от корня репозитория>
```
Примеры:
- bin
  - firmware.bin
  - esp8266
    - firmware.bin
  - esp32
    - firmware.bin
```
https://raw.githubusercontent.com/GyverLibs/GyverHub-example/main/bin/firmware.bin
https://raw.githubusercontent.com/GyverLibs/GyverHub-example/main/bin/esp8266/firmware.bin
https://raw.githubusercontent.com/GyverLibs/GyverHub-example/main/bin/esp32/firmware.bin
```

#### В релизах
```
https://github.com/<аккаунт>/<проект>/releases/latest/download/<файл>
```
Пример:
```
https://github.com/GyverLibs/GyverHub-example/releases/latest/download/firmware.bin
```

<a id="versions"></a>

## Версии
- v1.0

<a id="install"></a>
## Установка
- Библиотеку можно найти по названию **AutoOTA** и установить через менеджер библиотек в:
    - Arduino IDE
    - Arduino IDE v2
    - PlatformIO
- [Скачать библиотеку](https://github.com/GyverLibs/AutoOTA/archive/refs/heads/main.zip) .zip архивом для ручной установки:
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