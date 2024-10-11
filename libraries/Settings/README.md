[![latest](https://img.shields.io/github/v/release/GyverLibs/Settings.svg?color=brightgreen)](https://github.com/GyverLibs/Settings/releases/latest/download/Settings.zip)
[![PIO](https://badges.registry.platformio.org/packages/gyverlibs/library/Settings.svg)](https://registry.platformio.org/libraries/gyverlibs/Settings)
[![Foo](https://img.shields.io/badge/Website-AlexGyver.ru-blue.svg?style=flat-square)](https://alexgyver.ru/)
[![Foo](https://img.shields.io/badge/%E2%82%BD%24%E2%82%AC%20%D0%9F%D0%BE%D0%B4%D0%B4%D0%B5%D1%80%D0%B6%D0%B0%D1%82%D1%8C-%D0%B0%D0%B2%D1%82%D0%BE%D1%80%D0%B0-orange.svg?style=flat-square)](https://alexgyver.ru/support_alex/)
[![Foo](https://img.shields.io/badge/README-ENGLISH-blueviolet.svg?style=flat-square)](https://github-com.translate.goog/GyverLibs/Settings?_x_tr_sl=ru&_x_tr_tl=en)  

[![Foo](https://img.shields.io/badge/ПОДПИСАТЬСЯ-НА%20ОБНОВЛЕНИЯ-brightgreen.svg?style=social&logo=telegram&color=blue)](https://t.me/GyverLibs)

# Settings
Библиотека для создания простого веб-интерфейса настроек на esp8266/esp32
- Веб-приложение весит около 10кб и вшивается в программу в бинарном gzip виде без возни с файлами
- Удобный билдер панели управления из скетча
- Десяток типовых виджетов с возможностью объединения в группы и вложенные меню
- Система авторизации с разными правами для авторизованных юзеров и гостей
- Файловый менеджер и OTA (обновление по воздуху)
- Интеграция с библиотекой [GyverDB](https://github.com/GyverLibs/GyverDB) для полностью автоматического хранения данных
- Компактный бинарный протокол связи
- Легко адаптируется под любую библиотеку HTTP сервера, из коробки реализовано три версии: GyverHTTP, стандартный esp-WebServer, ESPAsyncWebserver
- Это [GyverHub](https://github.com/GyverLibs/GyverHub) на минималках 
- Исходник веб-приложения [здесь](https://github.com/GyverLibs/Settings-web)

> Есть [Android-приложение](https://github.com/GyverLibs/Settings-discover) для поиска устройств с библиотекой

![promo](/img/promo.png)

### Совместимость
ESP8266, ESP32

### Зависимости
- GTL v1.1.1+
- GyverDB v1.1.2+
- StringUtils v1.4.24+
- GyverHTTP v1.0.17+
- GSON v1.5.9+

> При установке из реестра PIO или Arduino IDE все зависимости установятся автоматически

<details>
<summary>platformio.ini</summary>

```ini
[env]
framework = arduino
lib_deps =
    GyverLibs/Settings
    ;esphome/ESPAsyncWebServer-esphome   ; для версии SettingsAsync
    ;esphome/ESPAsyncTCP-esphome         ; для версии SettingsAsync

[env:d1_mini]
platform = espressif8266
board = d1_mini
upload_speed = 921600
monitor_speed = 115200
monitor_filters = esp8266_exception_decoder, default
build_type = debug
board_build.filesystem = littlefs

[env:esp32dev]
monitor_speed = 115200
platform = espressif32
board = esp32dev
upload_speed = 921600
board_build.filesystem = littlefs

[env:esp32-c3]
monitor_speed = 115200
platform = espressif32
board = esp32dev
board_build.mcu = esp32c3
upload_speed = 2000000
board_build.f_cpu = 80000000L
board_build.filesystem = littlefs
```
</details>

## Содержание
- [Использование](#usage)
- [Версии](#versions)
- [Установка](#install)
- [Баги и обратная связь](#feedback)

<a id="usage"></a>

## Использование
### Как это работает
Вебморда является самостоятельным веб-приложением, написанным на html/css/js. Её файлы минифицированы, сжаты в gz и вшиты в код библиотеки как PROGMEM массив. В библиотеке настроен вебсервер, который отправляет файлы вебморды при заходе на IP платы в браузере. Лёгкий html файл подгружается каждый раз, а скрипты и стили кешируются браузером для ускорения загрузки. Вебморда общается с платой по http: например при загрузке запрашивает пакет с виджетами и прочей информацией, пакеты имеют формат бинарного json.

### Captive portal
Во всех трёх реализациях сервера из коробки настроен DNS для работы как Captive portal - если ESP работает в режиме точки доступа (AP или AP_STA), то при подключении к точке автоматически откроется окно браузера со страницей настроек.

### Приложение для поиска
Позволяет находить устройства с библиотекой в локальной сети и заменяет браузер, вебморда открывается сразу в приложении, кнопка назад возвращает к списку устройств. Чтобы удалить устройство - долгое удержание на нём на смартфоне или правой кнопкой мыши на ПК. Для поиска смартфон/ПК должны быть в одной локальной сети с устройством. В приложении должна быть указана корректная маска подсети (настраивается в роутере). Если в роутере она не менялась - то она там стандартная *255.255.255.0*, как и в приложении по умолчанию.
- [Скачать .apk](https://github.com/GyverLibs/Settings-discover/releases/latest/download/Settings.apk
) последней версии
- [Скачать .html](https://github.com/GyverLibs/Settings-discover/releases/latest/download/index.html) последней версии

> Требуется версия библиотеки v1.0.13+

### Билдер и виджеты
Пакет с виджетами собирается устройством в билдере - функция в программе, которая вызывается, когда приходит запрос от вебморды. Внутри билдера нужно вызвать методы виджетов в том порядке, в котором они должны находиться в вебморде.

```cpp
// минимальный код
SettingsGyver sett;

void build(sets::Builder& b) {
    // b.Input(...);
    // b.Button(...);
}

void setup() {
    // подключение к WiFi...

    sett.begin();
    sett.onBuild(build);
}

void loop() {
    sett.tick();
}
```

### ID виджета
У всех виджетов есть вариант функции с ID и без ID. ID виджета нужен для:
- Работа с подключенной базой данных на чтение и запись значений
- Отправка обновлений на виджет
- Разбор действий отдельно от вывода виджетов, чтобы разделить UI и обработку

ID в данной библиотеке задаётся числом, тип `size_t` (на ESP это 32-бит беззнаковое целое). Можно задавать ID:
- Просто числом вручную
- `enum`
- Хэш-строки из библиотеки StringUtils: `SH("my_input")` или `"my_input"_h`
- Хэш-ключи `DB_KEYS` из библиотеки GyverDB

> Если ID не задан - он будет присваиваться библиотекой автоматически (только для активных виджетов). Автоматический ID - это число от `UINT32_MAX`, уменьшается на 1 с каждым вызовом. Если автоматический ID совпадёт с каким то из вручную заданных - в вебморде высветится ошибка `Duplicated ID`

> Не рекомендуется задавать ID числом `0`, т.к. `b.build.id == 0` при запросе виджетов

```cpp
enum keys : size_t {
    my_inp = 1,
    button,
};

DB_KEYS(
    kk,
    my_inp,
    button
);

void build(sets::Builder& b) {
    b.Input("My input");                // без ID
    b.Input("my_inp"_h, "My input");    // хэш-строка
    b.Input(SH("my_inp"), "My input");  // хэш-строка
    b.Input(12, "My input");            // вручную числом
    b.Input(keys::my_inp, "My input");  // enum
    b.Input(kk::my_inp, "My input");    // GyverDB-хэш
}
```

### Взаимодействие с виджетами
Есть несколько способов взаимодействия с виджетами, т.е. отправки и получения значений:
- У виджета можно задать ID, по которому библиотека будет автоматически читать и писать данные в базу данных GyverDB
- К виджету можно подключить переменную, библиотека будет читать из неё значение и писать при изменении
- У виджета без ID и переменной будет значение по умолчанию (0 или пустая строка), но получить новое значение с вебморды можно из инфо о билде
- Активный виджет (значение можно менять из вебморды) возвращает `true` при изменении значения пользователем, также при клике по кнопке

```cpp
String str;
char cstr[20];

void build(sets::Builder& b) {
    // виджет без id и начального значения
    // При установке с вебморды получаем значение напрямую
    if (b.Input("My input")) {
        Serial.println(b.build.value);
    }

    // виджет без id с привязанной String-строкой
    // при установке с вебморды значение запишется в строку
    b.Input("My input", &str);
    b.Input("My input", AnyPtr(cstr, 20));  // для char-массивов

    // виджет с id без привязанной переменной
    // будет работать с базой данных по указанному ключу
    b.Input("my_inp"_h, "My input");

    // действие возвращается независимо от наличия id
    if (b.Button()) Serial.println("btn 1");
    if (b.Button("my_btn2"_h)) Serial.println("btn 2");
}
```

> Здесь `AnyPtr` - тип данных, принимающий указатель на любой встроенный тип (числа, строки). Для передачи ему char-буфера нужно явно вызвать конструктор с указанием размера буфера

```cpp
void build(sets::Builder& b) {
    // можно узнать, было ли действие по виджету
    if (b.build.isAction()) {
        Serial.print("Set: 0x");
        Serial.print(b.build.id, HEX);
        Serial.print(" = ");
        Serial.println(b.build.value);
    }
}
```

```cpp
// разделение UI и действий
void build(sets::Builder& b) {
    // вывод UI
    b.Input("my_inp"_h, "My input");
    b.Button("my_btn"_h, "My button");

    // обработка действий
    switch (b.build.id) {
        case "my_inp"_h:
            Serial.print("input: ");
            Serial.println(b.build.value);
            break;

        case "my_btn"_h:
            Serial.println("btn click");
            break;
    }
}
```

> Если ID виджета задан и переменная не привязана - будет использоваться БД. Если привязана переменная - будет использоваться она

### Тип данных Text
`Text` - это обёртка для текстовых данных из библиотеки [StringUtils](https://github.com/GyverLibs/StringUtils), см. документацию там. Text может конвертироваться в любые другие типы и сравниваться с ними, а также имеет множество инструментов для парсинга и поиска.

```cpp
Serial.println(b.build.value);  // печататься
b.build.value == 123;           // сравниваться
b.build.value == "123";         // сравниваться
b.build.value.toFloat();        // конвертироваться
byte b = b.build.value;         // авто-конвертироваться
```

### База данных
Библиотека интегрирована с [GyverDB](https://github.com/GyverLibs/GyverDB) - относительно быстрой базой данных для хранения данных любого типа. Settings автоматически читает и обновляет данные в БД, поэтому рекомендуется изучить как работать с БД на странице описания GyverDB. При использовании GyverDBFile база данных будет автоматически писаться в файл при изменениях, а файловая система позаботится об оптимальном износе flash памяти. При запуске рекомендуется инициализировать БД, указав ключи и соответствующие им начальные значения и типы. Эти значения будут записаны только в том случае, если запись в БД ещё не существует. В то же время автоматическое обновление БД работает только для существующих записей, т.е. Settings будет работать только с сущестующими ячейками и не создаст новых. Минимальный пример:

```cpp
// Подключить библиотеки и создать БД и Settings
#include <GyverDBFile.h>
#include <LittleFS.h>
GyverDBFile db(&LittleFS, "/data.db");

#include <SettingsESP.h>
SettingsESP sett("My Settings", &db);

// Объявить хэш-ключи БД через макрос. Это удобнее, чем "строки" и enum, 
// а также не боится изменения порядка или удаления ключей из середины списка
DB_KEYS(
    keys,
    input,
    slider
);

// билдер
void build(sets::Builder& b) {
    b.Input(kk::input);
    b.Slider(kk::slider);
}

void setup() {
    Serial.begin(115200);
    Serial.println();

    // WIFI
    WiFi.mode(WIFI_STA);
    WiFi.begin("WIFI_SSID", "WIFI_PASS");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.println(WiFi.localIP());

    // settings
    sett.begin();
    sett.onBuild(build);

    // запуск файловой системы
#ifdef ESP32
    LittleFS.begin(true); // format on fail
#else
    LittleFS.begin();
#endif

    // запуск БД и чтение из файла
    db.begin();
    
    // инициализация БД начальными значениями
    db.init(keys::input, "text");
    db.init(keys::slider, 30);
}

void loop() {
    sett.tick();
}
```

### Несколько БД
Можно использовать несколько баз данных, например одна для сохраняемых в память настроек, вторая для "временных" настроек, которые не нужно сохранять при перезагрузке: `GyverDBFile` сохраняет в файл, а обычная `GyverDB` - нет, живёт чисто в оперативной памяти. Переключаться между БД нужно в билдере таким образом, чтобы после смены БД шли только виджеты с ключами из этой БД. Например

```cpp
GyverDBFile db_flash(&LittleFS, "/data.db");
GyverDB db_ram;

void build(sets::Builder& b) {
    settings.attachDB(&db_ram);
    b.Input("input2"_h, "...");

    settings.attachDB(&db_flash);
    b.Input("input1"_h, "...");
}
```

> После выхода из билдера нужно оставлять подключенной ту БД, для которой нужны автоматические обновления, система не сможет обновляться одновременно с нескольких БД. Также нужно оставлять последней подключенной БД, которая пишет на флешку, чтобы система автоматически вызывала её тикер.

### Динамические виджеты
Виджеты собираются линейно, вызов функции виджета добавляет его в вебморду. Это означает, что виджеты можно выводить и динамически, особенно удобно это работает с автоматическим id. Например:

```cpp
int numbers[5];

void build(sets::Builder& b) {
    // вывод в цикле
    for (int i = 0; i < 5; i++) {
        b.Input();
    }
    
    // обработка действий также будет работать
    for (int i = 0; i < 5; i++) {
        if (b.Input()) Serial.println(b.build.value);
    }
    
    // массив number с привязанными переменными
    for (int i = 0; i < 5; i++) {
        b.Number(String("number #") + i, &numbers[i]);
    }

    // обработка и действий
    for (int i = 0; i < 5; i++) {
        if (b.Number(String("number #") + i, &numbers[i])) {
            Serial.print(String("number #") + i + ": ");
            Serial.println(numbers[i]);
        }
    }

    // можно и так
    for (int i = 0; i < 5; i++) {
        b.Number(String("number #") + i, &numbers[i]);

        if (b.wasSet()) {
            Serial.print(String("number #") + i + ": ");
            Serial.println(numbers[i]);
            b.clearSet();
        }
    }
}
```

Также можно динамически скрывать виджеты, например

```cpp
void build(sets::Builder& b) {
    if (flag) {
        b.Input();
        b.Slider();
        // ...
    }
}
```

Частым сценарием является открытие группы настроек с активацией режима, это можно сделать так:

```cpp
void build(sets::Builder& b) {
    if (b.Switch()) {
        b.reload(); // перезагрузить вебморду по клику на свитч
    }

    // здесь flag должен быть прочитан из БД или переменной
    if (flag) {
        b.Input();
        b.Slider();
        // ...
    }
}
```

Например с БД

```cpp
DB_KEYS(
    kk,
    mode_sw
);

void build(sets::Builder& b) {
    // запись в БД и перезагрузка
    if (b.Switch(kk::mode_sw)) b.reload();

    // чтение из БД
    if (db[kk::mode_sw]) {
        b.Input();
        b.Slider();
        // ...
    }
}
```

### Обновления
В системе реализован механизм обновлений - вебморда периодически запрашивает у устройства обновления. Если база данных подключена - то при изменениях значений в базе где то в программе библиотека автоматически отправит новые значения в вебморду (например если какое то значение изменилось при помощи кнопки). Примечание: если вебморда открыта одновременно с нескольких браузеров - обновления базы данных получит только тот из них, который запросил их первым. Также можно отправить свои значения, если база данных не подключена или не используется для каких-то виджетов. Для этого нужно подключить обработчик обновлений и вручную отправить данные по id виджета.

```cpp
void build(sets::Builder& b) {
    b.Label("lbl1"_h, "Random");
    b.Label("lbl2"_h, "millis()", "", sets::Colors::Red);
}
void update(sets::Updater& upd) {
    upd.update("lbl1"_h, random(100));
    upd.update("lbl2"_h, millis());
}

void setup() {
    sett.begin();
    sett.onBuild(build);
    sett.onUpdate(update);
}
```

### Статус
- Вебморда отслеживает статус устройства, при потере связи появится текст offline в заголовке страницы. После потери связи вебморда будет запрашивать информацию о виджетах, это очень удобно при разработке - например добавляем виджет, загружаем прошивку. За это время вебморда уже понимает что устройство оффлайн и при первом успешном подключении выводит актуальные виджеты.
- При изменении значений виджетов вебморда следит за доставкой пакета, при ошибке связи появится надпись **error*** у соответствующего виджета

### Контейнеры
Виджеты можно объединять в контейнеры. Контейнер нужно начать и закончить, так как пакет данных собирается линейно в целях оптимизации скорости и памяти. Метод `beginКонтейнер` всегда вернёт true для красоты организации кода в блоке условия:
```cpp
void build(sets::Builder& b) {
    if (b.beginGroup("Group 1")) {
        b.Input("input1"_h, "Text");

        b.endGroup();  // закрыть группу
    }
}
```

Второй вариант - у всех контейнеров есть парный класс, который сам откроет и закроет контейнер. Нужно создать объект с любым именем и передать ему билдер:
```cpp
void build(sets::Builder& b) {
    {
        sets::Group g(b, "Group 2");  // должен быть первым в блоке

        b.Input("input1"_h, "Text");
    }
}
```

Можно создавать вложенные меню. Указанный заголовок будет отображаться на кнопке и в заголовке страницы при входе на меню. Все виджеты и группы, находящиеся в блоке с меню, будут находиться на отдельной странице. Вложенность меню неограниченная.
```cpp
void build(sets::Builder& b) {
    b.Input("input1"_h, "Text 1");

    {
        sets::Menu g(b, "Submenu");

        b.Input("input2"_h, "Text 2");
    }
}
```

Отдельный тип контейнера - кнопки, внутри него можно добавлять только кнопки:
```cpp
void build(sets::Builder& b) {
    {
        sets::Buttons btns(b);

        // кнопка вернёт true при клике
        if (b.Button("btn1"_h, "Button 1")) {
            Serial.println("Button 1");
        }

        if (b.Button("btn2"_h, "Button 2", sets::Colors::Blue)) {
            Serial.println("Button 2");
        }
    }
}
```

### Авторизация
В системе предусмотрена авторизация: если в прошивке указать отличный от пустой строки пароль - вебморда будет работать в "гостевом" режиме: отображаются только разрешённые гостям виджеты, файловый менеджер и OTA скрыты и заблокированы. Для ввода пароля нужно зайти в меню (правая верхняя кнопка) и нажать на ключик. Серый ключик означает что авторизация отключена, зелёный - клиент авторизован, красный - неверный пароль. Пароль может содержать любые символы и иметь любую длину - в явном виде он не хранится и не передаётся. Пароль сохраняется в браузере и авторизация работает автоматически при перезагрузке страницы.

Для разделения админского и гостевого доступа предусмотрен виртуальный контейнер Guest. Если пароль установлен и клиент не авторизован - он будет видеть только виджеты из гостевых контейнеров. Для корректной работы гостевой контейнер не должен прерываться обычными контейнерами. Пример:

```cpp
if (b.beginGroup("Group 1")) {
    // гости не видят
    b.Pass(kk::pass, "Password");

    // виджеты, которые видят гости и админы
    {
        sets::GuestAccess g(b);
        b.Input(kk::uintw, "uint");
        b.Input(kk::intw, "int");
        b.Input(kk::int64w, "int 64");
    }

    // гости не видят
    {
        sets::Menu m(b, "sub sub");
        b.Label(kk::lbl2, "millis()", "", sets::Colors::Red);
    }

    b.endGroup();
}
```
В гостевой контейнер можно поместить несколько обычных контейнеров, например групп.

> Примечание: если вложенное меню закрыто от гостей, но содержит ещё одно вложенное меню - кнопка открытия меню будет отображаться, но само меню будет пустым

### Иконки лейблов
Можно использовать emoji, они неплохо смотрятся в меню. Например с [удобного сайта](https://symbl.cc/ru/emoji/)

```cpp
b.Input(kk::intw, "🔈Громкость");
```

## Описание классов
- `SettingsGyver` (*SettingsGyver.h*) - на вебсервере GyverHTTP
- `SettingsESP` (*SettingsESP.h*) - на стандартном вебсервере ESP
- `SettingsAsync` (*SettingsAsync.h*) - на асинхронном ESPAsyncWebserver

### Настройки компиляции
```cpp
#define SETT_NO_DB  // полностью отключить поддержку GyverDB
```

### SettingsBase/SettingsGyver/SettingsESP/SettingsAsync
```cpp
Settings(const String& title = "", GyverDB* db = nullptr);

// установить пароль на вебморду. Пустая строка "" чтобы отключить
void setPass(Text pass);

// перезагрузить страницу. Можно вызывать где угодно + в обработчике update
void reload();

// установить заголовок страницы
void setTitle(const String& title);

// установить период обновлений (умолч. 2500мс), 0 чтобы отключить
void setUpdatePeriod(uint16_t prd);

// подключить базу данных
void attachDB(GyverDB* db);

// использовать автоматические обновления из БД (при изменении записи новое значение отправится в браузер)
void useAutoUpdates(bool use);

// обработчик билда
void onBuild(BuildCallback cb);

// обработчик обновлений
void onUpdate(UpdateCallback cb);

// тикер, вызывать в родительском классе
void tick();
```

### Builder
```cpp
// инфо о билде
Build build;

// авто-ID следующего виджета
size_t nextID();

// указатель на текущий SettingsXxx
void* thisSettings();

// перезагрузить страницу (вызывать в действии, например if (...click() b.reload()))
void reload();

// было действие с каким-то из виджетов выше
bool wasSet();

// сбросить флаг чтения wasSet
void clearSet();

// КОНТЕЙНЕРЫ
// разрешить неавторизованным клиентам следующий код
bool beginGuest();

// запретить неавторизованным клиентам
void endGuest();

// группа
bool beginGroup(Text title = Text());
void endGroup();

// вложенное меню
bool beginMenu(Text title);
void endMenu();

// ряд кнопок
bool beginButtons();
void endButtons();

// ВИДЖЕТЫ
// ПАССИВНЫЕ
// ================= LABEL =================
// текстовое значение, может обновляться по id
void Label(size_t id, Text label = "", Text text = Text(), uint32_t color = SETS_DEFAULT_COLOR);
void Label(size_t id, Text label, Text text, sets::Colors color);
void Label(Text label = "", Text text = Text(), uint32_t color = SETS_DEFAULT_COLOR);
void Label(Text label, Text text, sets::Colors color);

// ================= LED =================
// светодиод (value 1 включен - зелёный, value 0 выключен - красный)
void LED(size_t id, Text label, bool value);
void LED(size_t id, Text label = "");
void LED(Text label, bool value);
void LED(Text label = "");

// ================= TEXT =================
// текстовый абзац
void Paragraph(size_t id, Text label = "", Text text = Text());
void Paragraph(Text label = "", Text text = Text());

// active

// ================= INPUT =================
// ввод текста и цифр [результат - строка], подключаемая переменная - любой тип
bool Input(size_t id, Text label = "", AnyPtr value = nullptr);
bool Input(Text label = "", AnyPtr value = nullptr);

// ================= NUMBER =================
// ввод цифр [результат - строка], подключаемая переменная - любой тип
bool Number(size_t id, Text label = "", AnyPtr value = nullptr);
bool Number(Text label = "", AnyPtr value = nullptr);

// ================= PASS =================
// ввод пароля [результат - строка], подключаемая переменная - любой тип
bool Pass(size_t id, Text label = "", AnyPtr value = nullptr);
bool Pass(Text label = "", AnyPtr value = nullptr);

// ================= COLOR =================
// ввод цвета [результат - 24-бит DEC число], подключаемая переменная - uint32_t
bool Color(size_t id, Text label = "", uint32_t* value = nullptr);
bool Color(Text label = "", uint32_t* value = nullptr);

// ================= SWITCH =================
// переключатель [результат 1/0], подключаемая переменная - bool
bool Switch(size_t id, Text label = "", bool* value = nullptr);
bool Switch(Text label = "", bool* value = nullptr);

// ================= DATE =================
// дата [результат - unix секунды], подключаемая переменная - uint32_t
bool Date(size_t id, Text label = "", uint32_t* value = nullptr);
bool Date(Text label = "", uint32_t* value = nullptr);

// ================= TIME =================
// время [результат - секунды с начала суток], подключаемая переменная - uint32_t
bool Time(size_t id, Text label = "", uint32_t* value = nullptr);
bool Time(Text label = "", uint32_t* value = nullptr);

// ================= DATETIME =================
// дата и время [результат - unix секунды], подключаемая переменная - uint32_t
bool DateTime(size_t id, Text label = "", uint32_t* value = nullptr);
bool DateTime(Text label = "", uint32_t* value = nullptr);

// ================= SLIDER =================
// слайдер [результат - число], подключаемая переменная - любой тип
bool Slider(size_t id, Text label = "", float min = 0, float max = 100, float step = 1, Text unit = Text(), AnyPtr value = nullptr);
bool Slider(Text label = "", float min = 0, float max = 100, float step = 1, Text unit = Text(), AnyPtr value = nullptr);

// ================= SELECT =================
// опции разделяются ; [результат - индекс (число)], подключаемая переменная - uint8_t
bool Select(size_t id, Text label, Text options, uint8_t* value = nullptr);
bool Select(Text label, Text options, uint8_t* value = nullptr);

// ================= BUTTON =================
// кнопку можно добавлять как внутри контейнера кнопок, так и как одиночный виджет
bool Button(size_t id, Text label = "", uint32_t color = SETS_DEFAULT_COLOR);
bool Button(Text label = "", uint32_t color = SETS_DEFAULT_COLOR);

bool Button(size_t id, Text label, sets::Colors color);
bool Button(Text label, sets::Colors color);

// misc
// окно подтверждения, для активации отправь пустой update на его id или update с текстом подтверждения
bool Confirm(size_t id, Text label = "");
```

- `Text` - универсальный текстовый формат, принимает строки в любом виде. При указании `value` отличным от стандартного будет отправлено его значение. Иначе будет отправлено значение из БД, если она подключена. Если в качестве значения нужно число - используйте конструктор `Value`, например `b.Color("col", "Color", Value(my_color));`, где `my_color` это `uint32_t`.
- `AnyPtr` - указатель на переменную любого типа из списка: float, double, любой целочисленный, String, AnyPtr(char[], size_t len)

### Build
Инфо о билде
```cpp
// тип билда
const Type type;

// клиент авторизован
const bool granted;

// id виджета (действие)
const size_t id;

// значение виджета (действие)
const Text value;

// тип - сборка виджетов
bool isBuild();

// тип - действие (обработка клика или значения)
bool isAction();
```

### Контейнеры
```cpp
// контейнер гостевого доступа
class GuestAccess(Builder& b);

// контейнер группы виджетов
class Group(Builder& b, Text title = Text());

// контейнер вложенного меню
class Menu(Builder& b, Text title);

// контейнер кнопок
class Buttons(Builder& b);
```

### Updater
```cpp
// пустой апдейт (например для вызова Confirm)
void update(size_t id);

// апдейт с текстом
void update(size_t id, Text value);

// апдейт с float
void update(size_t id, float value, uint8_t dec = 2);

// апдейт с числом
void update(size_t id, <любой численный тип> value);
```

<a id="versions"></a>

## Версии
- v1.0
- v1.0.2
  - Добавлен виджет Confirm (всплывающее окно подтверждения)
  - Кастомные всплывающие окна для Input (Input теперь работает на просмотре AP WiFi точки на Xiaomi)
- v1.0.5
  - Добавлен виджет LED
  - Добавлен файловый менеджер
  - Добавлено ОТА обновление
  - Добавлена авторизация и гостевой фильтр виджетов
  - Новый стиль для Select
- v1.1.0
  - Добавлен виджет Number
  - Создание виджета без ID (автоматический ID)
  - Создание виджета без лейбла (будет равен типу виджета)
  - Привязка внешней переменной к виджету на чтение и запись
  - Редактор текстовых файлов в менеджере файлов
  - Убран лаг с прошлым состоянием вебморды при обновлении страницы
  - Возможность полностью убрать поддержку GyverDB дефайном
  - Мелкие фиксы стилей
  - Методы build(), value(), id() теперь являются членами (build, value, id) для краткости

<a id="install"></a>
## Установка
- Библиотеку можно найти по названию **Settings** и установить через менеджер библиотек в:
    - Arduino IDE
    - Arduino IDE v2
    - PlatformIO
- [Скачать библиотеку](https://github.com/GyverLibs/Settings/archive/refs/heads/main.zip) .zip архивом для ручной установки:
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
