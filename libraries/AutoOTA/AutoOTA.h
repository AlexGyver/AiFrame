#pragma once
#include <Arduino.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <Updater.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureBearSSL.h>
#include <flash_hal.h>
#elif defined(ESP32)
#include <Update.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#endif

#if CONFIG_IDF_TARGET_ESP32
#define GOTA_PLATFORM "ESP32"
#elif CONFIG_IDF_TARGET_ESP32S2
#define GOTA_PLATFORM "ESP32-S2"
#elif CONFIG_IDF_TARGET_ESP32S3
#define GOTA_PLATFORM "ESP32-S3"
#elif CONFIG_IDF_TARGET_ESP32C3
#define GOTA_PLATFORM "ESP32-C3"
#elif CONFIG_IDF_TARGET_ESP32C6
#define GOTA_PLATFORM "ESP32-C6"
#elif CONFIG_IDF_TARGET_ESP32H2
#define GOTA_PLATFORM "ESP32-H2"
#elif defined(ESP8266)
#define GOTA_PLATFORM "ESP8266"
#else
#error "OTA platform is not supported"
#endif

#define GOTA_TIMEOUT 2500

class AutoOTA {
   public:
    AutoOTA(const char* cur_ver, const char* path, const char* host = "raw.githubusercontent.com", uint16_t port = 443) : _cur_ver(cur_ver), _path(path), _host(host), _port(port) {}

    enum class Error : uint8_t {
        None,
        Connect,
        Timeout,
        HTTP,
        NoVersion,
        NoPlatform,
        NoPath,
        NoUpdates,
        NoFile,
        OtaStart,
        OtaEnd,
    };

    // текущая версия
    const char* version() {
        return _cur_ver;
    }

    // проверить обновления. Можно передать строки для записи информации
    bool checkUpdate(String* version = nullptr, String* notes = nullptr) {
#if defined(ESP8266)
        BearSSL::WiFiClientSecure client;
        client.setBufferSizes(1024, 512);
#elif defined(ESP32)
        WiFiClientSecure client;
#endif
        client.setInsecure();

        if (!_request(client, _jsonReq())) return false;

        String ver = _readKey(client, "version");
        if (!ver.length()) return _err = Error::NoVersion, false;
        if (version) *version = ver;
        if (ver == _cur_ver) return _err = Error::NoUpdates, false;
        if (notes) *notes = _readKey(client, "notes");
        _has_update = true;
        _err = Error::None;
        return true;
    }

    // есть обновление. Вызывать после проверки. Само сбросится в false
    bool hasUpdate() {
        return _has_update ? _has_update = false, true : false;
    }

    // обновить прошивку из loop
    void update() {
        _ota_f = true;
    }

    // обновить прошивку сейчас и перезагрузить чип
    bool updateNow() {
#if defined(ESP8266)
        BearSSL::WiFiClientSecure client;
#elif defined(ESP32)
        WiFiClientSecure client;
#endif
        client.setInsecure();

        if (!_request(client, _jsonReq())) return false;
        String bin;

        while (true) {
            delay(0);
            String chip = _readKey(client, "chipFamily");
            if (!chip.length()) return _err = Error::NoPlatform, false;

            if (chip != GOTA_PLATFORM) continue;

            bin = _readKey(client, "path");
            if (!bin.length()) return _err = Error::NoPath, false;

            break;
        }

        // flush
        while (client.available()) {
            delay(0);
            client.read();
        }
        client.stop();

        // req file
        bin = F("GET ") + bin + F(" HTTP/1.1\r\n\r\n");
        if (!_request(client, bin)) return false;

        if (!_waitClient(client)) return _err = Error::NoFile, false;

#ifdef ESP8266
        size_t ota_size = (size_t)((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000);
#else
        size_t ota_size = UPDATE_SIZE_UNKNOWN;
#endif

        if (!Update.begin(ota_size, U_FLASH)) {
            return _err = Error::OtaStart, false;
        }

        while (_waitClient(client)) Update.write(client);

        if (!Update.end(true)) {
            return _err = Error::OtaEnd, false;
        }

        ESP.restart();
        _err = Error::None;
        return true;
    }

    // тикер, вызывать в loop. Вернёт true при попытке обновления
    bool tick() {
        if (_ota_f) {
            _ota_f = false;
            updateNow();
            return true;
        }
        return false;
    }

    // есть ошибка
    bool hasError() {
        return _err != Error::None;
    }

    // прочитать ошибку
    Error getError() {
        return _err;
    }

   private:
    const char* _cur_ver;
    const char* _path;
    const char* _host;
    uint16_t _port;
    Error _err = Error::None;
    bool _has_update = false;
    bool _ota_f = false;

    String _jsonReq() {
        String req(F("GET /"));
        req += _path;
        req += F(" HTTP/1.1\r\nHost: ");
        req += _host;
        req += F("\r\n\r\n");
        return req;
    }

    String _readKey(Stream& stream, const String& key) {
        while (stream.available()) {
            stream.readStringUntil('"');
            String thisKey = stream.readStringUntil('"');
            if (thisKey == key) {
                stream.readStringUntil('"');
                return stream.readStringUntil('"');
            }
        }
        return String();
    }

    bool _waitClient(Client& client) {
        if (!client.connected()) return false;
        if (!client.available()) {
            uint32_t ms = millis();
            while (!client.available()) {
                delay(0);
                if (millis() - ms >= GOTA_TIMEOUT) {
                    _err = Error::Timeout;
                    return false;
                }
                if (!client.connected()) return false;
            }
        }
        return true;
    }

    bool _request(Client& client, const String& req) {
        if (!client.connected()) client.connect(_host, _port);

        if (client.connected()) {
            client.print(req);

            if (!_waitClient(client)) return false;

            client.readStringUntil(' ');  // HTTP/1.1 CODE

            if (client.parseInt() == 200) {
                while (client.available() && client.connected()) {
                    delay(0);
                    if (client.readStringUntil('\n').length() == 1) {  // \r\n
                        return true;
                    }
                }
            }

            _err = Error::HTTP;
        } else {
            _err = Error::Connect;
        }
        return false;
    }
};