#pragma once
#include <Arduino.h>
#include <Client.h>

#ifndef __AVR__
#include <functional>
#endif

#include "HeadersParser.h"
#include "StreamReader.h"
#include "cfg.h"

#define HC_DEF_TIMEOUT 2000     // таймаут по умолчанию
#define HC_HEADER_BUF_SIZE 150  // буфер одной строки хэдера
#define HC_FLUSH_BLOCK 64       // блок очистки
#define HC_BOUNDARY "----GyverHttpBoundary123454321"

// #define HC_USE_LOG Serial

#ifdef HC_USE_LOG
#define HC_LOG(x) HC_USE_LOG.println(x)
#else
#define HC_LOG(x)
#endif

namespace ghttp {

class Client : public Print {
   public:
    // билдер form data
    class FormData {
        friend class Client;

       public:
        void add(const Text& name, const Text& filename, const Text& type, const Text& data) {
            if (_first) s += F("--" HC_BOUNDARY);
            _first = false;
            clrf();
            s += F("Content-Disposition: form-data; name=\"");
            name.addString(s);
            s += '"';
            if (filename.length()) {
                s += F("; filename=\"");
                filename.addString(s);
                s += '"';
            }
            clrf();
            if (type.length()) {
                s += F("Content-Type: ");
                type.addString(s);
                clrf();
            }
            clrf();
            data.addString(s);
            clrf();
            s += F("--" HC_BOUNDARY);
        }

       private:
        String s;
        bool _first = true;
        void clrf() {
            s += "\r\n";
        }
    };

    // билдер заголовков
    class Headers {
        friend class Client;

       public:
        void add(const Text& name, const Text& value) {
            name.addString(headers);
            headers += ": ";
            value.addString(headers);
            headers += "\r\n";
        }

        operator Text() {
            return headers;
        }

       private:
        String headers;
    };

    // парсер ответа
    class Response {
       public:
        Response() {}
        Response(const String& type, Stream* stream, size_t len, bool chunked, uint16_t code) : _type(type), _reader(stream, len, chunked), _code(code) {}

        // тип контента
        Text type() const {
            return _type;
        }

        // тело ответа
        StreamReader& body() {
            return _reader;
        }

        // код ответа
        uint16_t code() {
            return _code;
        }

        // ответ существует
        operator bool() {
            return _reader;
        }

       private:
        String _type;
        StreamReader _reader;
        uint16_t _code;
    };

   private:
#ifdef __AVR__
    typedef void (*ResponseCallback)(Response& resp);
#else
    typedef std::function<void(Response& resp)> ResponseCallback;
#endif

   public:
    Client(::Client& client, const char* host, uint16_t port) : client(client), _host(host), _port(port) {
        setTimeout(HC_DEF_TIMEOUT);
    }
    Client(::Client& client, const IPAddress& ip, uint16_t port) : client(client), _host(nullptr), _ip(ip), _port(port) {
        setTimeout(HC_DEF_TIMEOUT);
    }

    size_t write(uint8_t data) {
        if (!client.connected()) {
            _init();
            return 0;
        }
        // client.flush();
        _waiting = 1;
        _lastSend = millis();
        return client.write(data);
    }
    size_t write(const uint8_t* buffer, size_t size) {
        if (!client.connected()) {
            _init();
            return 0;
        }
        size_t w = client.write(buffer, size);
        // client.flush();
        _waiting = 1;
        _lastSend = millis();
        return w;
    }

    // ==========================

    // установить новый хост и порт
    void setHost(const char* host, uint16_t port) {
        stop();
        _host = host;
        _port = port;
    }

    // установить новый хост и порт
    void setHost(const IPAddress& ip, uint16_t port) {
        setHost(nullptr, port);
        _ip = ip;
    }

    // установить новый клиент для связи
    void setClient(::Client& client) {
        stop();
        this->client = client;
        client.setTimeout(_timeout);
    }

    // установить таймаут ответа сервера, умолч. 2000 мс
    void setTimeout(uint16_t tout) {
        client.setTimeout(tout);
        _timeout = tout;
    }

    // обработчик ответов, требует вызова tick() в loop()
    void onResponse(ResponseCallback cb) {
        _resp_cb = cb;
    }

    // ==========================

    // подключиться
    bool connect() {
        if (!client.connected()) {
            _host ? client.connect(_host, _port) : client.connect(_ip, _port);
        }
        return client.connected();
    }

    // отправить запрос
    bool request(const Text& path, const Text& method, const Text& headers, FormData& data) {
        data.s += "--";
        return request(path, method, headers, (uint8_t*)data.s.c_str(), data.s.length(), true);
    }

    // отправить запрос
    bool request(const Text& path, const Text& method, const Text& headers, const Text& payload) {
        return request(path, method, headers, (uint8_t*)payload.str(), payload.length());
    }

    // отправить запрос
    bool request(const Text& path, const Text& method = "GET", const Text& headers = Text(), const uint8_t* payload = nullptr, size_t length = 0, bool formdata = 0) {
        if (!beginSend()) return 0;

        String req;
        req.reserve(50 + path.length() + headers.length());
        method.addString(req);
        req += ' ';
        path.addString(req);
        req += F(" HTTP/1.1\r\nHost: ");
        if (_host) req += _host;
        else req += _ip.toString();
        req += F("\r\n");
        headers.addString(req);
        if (formdata) {
            req += F("Content-Type: multipart/form-data; boundary=" HC_BOUNDARY "\r\n");
        }
        if (payload && length) {
            req += F("Content-Length: ");
            req += length;
            req += F("\r\n");
        }
        req += F("\r\n");
        print(req);
        if (payload && length) write(payload, length);
        return 1;
    }

    // начать отправку. Дальше нужно вручную print
    bool beginSend() {
        flush();
        return connect();
    }

    // клиент ждёт ответа
    bool isWaiting() {
        if (!client.connected()) {
            _init();
            return 0;
        }
        return _waiting;
    }

    // есть ответ от сервера (асинхронно)
    bool available() {
        return (isWaiting() && client.available());
    }

    // дождаться и прочитать ответ сервера (по available если long poll)
    Response getResponse(HeadersCollector* collector = nullptr) {
        if (!isWaiting()) return Response();

        if (!_wait()) {
            flush();
            return Response();
        }

        String lineStr = client.readStringUntil('\n');
        Text lines[3];
        Text(lineStr).split(lines, 3, ' ');

        HeadersParser headers(client, HC_HEADER_BUF_SIZE, collector);

        if (headers) {
            _close = headers.close;
            _waiting = 0;
            return Response(headers.contentType, &client, headers.length, headers.chunked, lines[1].toInt());
        } else {
            flush();
            return Response();
        }
    }

    // тикер, вызывать в loop для работы с коллбэком
    void tick() {
        if (available() && _resp_cb) {
            Response resp = getResponse();
            if (resp) _resp_cb(resp);
        }
    }

    // остановить клиента
    void stop() {
        HC_LOG("client stop");
        client.stop();
        _init();
    }

    // пропустить ответ, снять флаг ожидания, остановить если connection close
    void flush() {
        if (client.connected()) {
            _wait();
            uint8_t bytes[HC_FLUSH_BLOCK];
            while (client.available()) {
                delay(1);
                GHTTP_ESP_YIELD();
                client.readBytes(bytes, min(client.available(), HC_FLUSH_BLOCK));
            }
            if (_close) {
                HC_LOG("connection close");
                client.stop();
            }
        }
        _init();
    }

    ::Client& client;

   private:
    ResponseCallback _resp_cb = nullptr;
    const char* _host = nullptr;
    IPAddress _ip;
    uint16_t _port;
    uint16_t _timeout;
    uint32_t _lastSend;
    bool _close = 0;
    bool _waiting = 0;

    void _init() {
        _close = 0;
        _waiting = 0;
    }
    bool _wait() {
        if (!_waiting) return 0;
        while (!client.available()) {
            delay(1);
#ifdef ESP8266
            optimistic_yield(5000);
#endif
            GHTTP_ESP_YIELD();

            if (millis() - _lastSend >= _timeout) {
                HC_LOG("client timeout");
                stop();
                return 0;
            }
            if (!client.connected()) {
                HC_LOG("client disconnected");
                return 0;
            }
        }
        return 1;
    }
};

}  // namespace ghttp