#pragma once
#include <Arduino.h>
#include <Client.h>
#include <StringUtils.h>

#include "HeadersParser.h"
#include "StreamReader.h"
#include "StreamWriter.h"
#include "cfg.h"

#ifndef __AVR__
#include <functional>
#endif

#if defined(ESP8266) || defined(ESP32)
#include <FS.h>
#endif

#define HS_HEADER_BUF_SIZE 150  // буфер одной строки хэдера
#define HS_BLOCK_SIZE 256       // размер блока выгрузки из файла и PROGMEM
#define HS_FLUSH_BLOCK 64       // блок очистки
#define HS_CACHE_PRD "604800"   // период кеширования

namespace ghttp {

class ServerBase {
   public:
    class Headers {
        friend class ServerBase;
        Headers() {
            s.reserve(200);
        }
        void begin(uint16_t code) {
            if (_started) return;
            _started = true;
            s += F("HTTP/1.1 ");
            s += code;
            if (code == 200) s += (code == 200) ? F(" OK\r\n") : F(" ERROR\r\n");
        }
        void cache(bool enabled) {
            checkStart();
            if (enabled) {
                s += F("Cache-Control: max-age=" HS_CACHE_PRD "\r\n");
            } else {
                s += F(
                    "Cache-Control: no-cache, no-store, must-revalidate\r\n"
                    "Pragma: no-cache\r\n"
                    "Expires: 0\r\n");
            }
        }
        void type(const Text& t) {
            checkStart();
            s += F("Content-Type: ");
            if (t) t.addString(s);
            else s += F("text/plain");
            clrf();
        }
        void gzip(bool enabled) {
            checkStart();
            if (enabled) s += F("Content-Encoding: gzip\r\n");
        }
        void cors(bool use = true) {
            checkStart();
            if (use) {
                s += F(
                    "Access-Control-Allow-Origin:*\r\n"
                    "Access-Control-Allow-Private-Network: true\r\n"
                    "Access-Control-Allow-Methods:*\r\n");
            }
        }
        void length(size_t len) {
            checkStart();
            s += F("Content-Length: ");
            s += len;
            clrf();
        }

       public:
        // код ответа сервера
        Headers(uint16_t code) {
            s.reserve(200);
            begin(code);
        }

        // добавить хэдер
        void add(const Text& name, const Text& value) {
            checkStart();
            name.addString(s);
            s += F(": ");
            value.addString(s);
            clrf();
        }

       private:
        String s;
        bool _started = false;
        void clrf() {
            s += F("\r\n");
        }
        void checkStart() {
            if (!_started) begin(200);
        }
    };

    class Request {
       public:
        Request(const Text& method, const Text& url, Stream* stream, size_t len, bool chunked = false) : _reader(stream, len, chunked), _method(method), _url(url) {
            _q = _url.indexOf('?');
        }

        // метод запроса
        const Text& method() const {
            return _method;
        }

        // полный урл
        const Text& url() const {
            return _url;
        }

        // путь (без параметров)
        Text path() const {
            return (_q > 0) ? _url.substring(0, _q) : _url;
        }

        // получить значение параметра по ключу
        Text param(const Text& key) const {
            if (_q < 0) return Text();

            Text params = _url.substring(_q + 1);
            int p = 0;

            while (1) {
                p = params.indexOf(key, p);
                if (p < 0) return Text();

                p += key.length();
                if (p == params.length() || params[p] == '&') {
                    return Text("", 0);
                }
                if (params[p] == '=') {
                    p++;
                    break;
                }
            }
            int end = params.indexOf('&', p);
            if (end < 0) end = params.length();
            return params.substring(p, end);
        }

        // получить тело запроса. Может выводиться в Print
        StreamReader& body() {
            return _reader;
        }

       private:
        StreamReader _reader;
        const Text _method;
        const Text _url;
        int16_t _q = -1;
    };

#ifdef __AVR__
    typedef void (*RequestCallback)(Request req);
#else
    typedef std::function<void(Request req)> RequestCallback;
#endif

    // ==================== SERVER ====================
   public:
    // начать ответ. В Headers можно указать кастомные хэдеры. Отправка через send/print
    void beginResponse(Headers& resp) {
        _beginResponse(resp, false);
    }

    // начать ответ. Отправка через send/print
    void beginResponse(uint16_t code = 200) {
        Headers resp(code);
        _beginResponse(resp, false);
    }

    // доступ к клиенту для отправки
    ::Client* client() {
        return _clientp;
    }

    // подключить обработчик запроса
    void onRequest(RequestCallback callback) {
        _req_cb = callback;
    }

    // отправить клиенту и завершить сеанс. Должно быть единственным ответом, использовать без beginResponse
    void sendSingle(const Text& text, uint16_t code = 200, Text type = Text()) {
        if (!_clientp || _respStarted) return;

        Headers resp(code);
        resp.type(type);
        resp.length(text.length());
        _beginResponse(resp, true);
        _clientp->print(text);
        _clientp = nullptr;
    }

    // отправить клиенту. Можно вызывать несколько раз подряд
    void send(const Text& text, uint16_t code, Text type = Text()) {
        if (!_clientp) return;

        if (!_respStarted) {
            Headers resp(code);
            resp.type(type);
            _beginResponse(resp, true);
        }
        _clientp->print(text);
    }

    // отправить клиенту. Можно вызывать несколько раз подряд
    void send(const Text& text) {
        if (!_clientp) return;

        if (!_respStarted) {
            send(text, 200);
        } else {
            if (!_contentBegin) {
                _contentBegin = true;
                _clientp->println();
            }
            _clientp->print(text);
        }
    }

    // отправить клиенту. Можно вызывать несколько раз подряд
    void print(Printable& p) {
        if (!_clientp) return;

        if (!_respStarted) {
            Headers resp(200);
            _beginResponse(resp, true);
        }
        _clientp->print(p);
    }

    // отправить клиенту код. Должно быть единственным ответом
    void send(uint16_t code) {
        if (!_clientp) return;

        _flush();
        if (!_respStarted) {
            Headers resp(code);
            _beginResponse(resp, true);
        }
        _respStarted = true;
        _clientp = nullptr;
    }

#ifdef FS_H
    // отправить файл
    void sendFile(File& file, Text type = Text(), bool cache = false, bool gzip = false) {
        if (!_clientp) return;
        StreamWriter writer(&file, file.size());
        _sendFile(writer, type, cache, gzip);
    }
#endif

    // отправить файл-строку как текст
    void sendFile(const Text& text, Text type = Text(), bool cache = false) {
        if (!_clientp) return;
        StreamWriter writer(text.str(), text.length(), text.pgm());
        _sendFile(writer, type, cache, false);
    }

    // отправить файл из буфера
    void sendFile(const uint8_t* buf, size_t len, Text type = Text(), bool cache = false, bool gzip = false) {
        if (!_clientp) return;
        StreamWriter writer(buf, len);
        _sendFile(writer, type, cache, gzip);
    }

    // отправить файл из PROGMEM
    void sendFile_P(const uint8_t* buf, size_t len, Text type = Text(), bool cache = false, bool gzip = false) {
        if (!_clientp) return;
        StreamWriter writer(buf, len, true);
        _sendFile(writer, type, cache, gzip);
    }

    // отправить файл-строку из PROGMEM
    void sendFile_P(const char* pstr, Text type = Text(), bool cache = false) {
        if (!_clientp) return;
        StreamWriter writer(pstr, strlen_P(pstr), true);
        _sendFile(writer, type, cache, false);
    }

    // пометить запрос как выполненный
    void handle() {
        _respStarted = true;
    }

    // использовать CORS хэдеры (умолч. включено)
    void useCors(bool use) {
        _cors = use;
    }

    // получить mime тип файла по его пути
    const __FlashStringHelper* getMime(const Text& path) {
        int16_t pos = path.lastIndexOf('.');
        if (pos > 0) {
            switch (path.substring(pos + 1).hash()) {
                case su::SH("avi"): return F("video/x-msvideo");
                case su::SH("bin"): return F("application/octet-stream");
                case su::SH("bmp"): return F("image/bmp");
                case su::SH("css"): return F("text/css");
                case su::SH("csv"): return F("text/csv");
                case su::SH("gz"): return F("application/gzip");
                case su::SH("gif"): return F("image/gif");
                case su::SH("html"): return F("text/html");
                case su::SH("js"): return F("text/javascript");
                case su::SH("json"): return F("application/json");
                case su::SH("png"): return F("image/png");
                case su::SH("svg"): return F("image/svg+xml");
                case su::SH("wav"): return F("audio/wav");
                case su::SH("xml"): return F("application/xml");
                case su::SH("jpeg"):
                case su::SH("jpg"):
                    return F("image/jpeg");
            }
        }
        return F("text/plain");
    }

    // обработать запрос
    void handleRequest(::Client& client, HeadersCollector* collector = nullptr) {
        String lineStr = client.readStringUntil('\n');
        Text lines[3];
        size_t n = Text(lineStr).split(lines, 3, ' ');
        if (n != 3) return;

        HeadersParser headers(client, HS_HEADER_BUF_SIZE, collector);

        if (!headers || !_req_cb) return send(400);

        _clientp = &client;
        _respStarted = false;
        _contentBegin = false;

        if (headers.contentType.startsWith(F("multipart")) && headers.length) {
            bool eol = false;
            size_t boundlen = 0;
            while (client.connected()) {
                GHTTP_ESP_YIELD();
                String s = client.readStringUntil('\n');
                if (!s.length() || s[s.length() - 1] != '\r') break;

                if (!boundlen) boundlen = s.length();
                headers.length -= s.length() + 1;  // + \n
                if (s.length() == 1) {
                    eol = 1;
                    break;
                }
            }
            if (eol && headers.length >= boundlen + 2 + 3) {
                _req_cb(Request(lines[0], lines[1], &client, headers.length - (boundlen + 2 + 3)));  // \r\n + --
            }
            _flush();
        } else {
            _req_cb(Request(lines[0], lines[1], &client, headers.length, headers.chunked));
        }

        if (!_respStarted) send(500);
        _clientp = nullptr;
    }

   private:
    RequestCallback _req_cb = nullptr;
    ::Client* _clientp = nullptr;
    bool _respStarted = false;
    bool _contentBegin = false;
    bool _cors = true;

    void _beginResponse(Headers& resp, bool lastHeader) {
        if (!_clientp || _respStarted) return;

        _flush();
        resp.cors(_cors);
        if (lastHeader) _clientp->println(resp.s);
        else _clientp->print(resp.s);
        _contentBegin = lastHeader;
        _respStarted = true;
    }
    void _sendFile(StreamWriter& writer, const Text& type, bool cache, bool gzip) {
        _flush();
        writer.setBlockSize(HS_BLOCK_SIZE);

        if (!_contentBegin) {
            Headers resp;
            if (!_respStarted) {
                resp.begin(200);
                resp.cors(_cors);
            }
            resp.length(writer.length());
            resp.type(type);
            resp.cache(cache);
            resp.gzip(gzip);
            _clientp->println(resp.s);

            _clientp->print(writer);
        }
        _respStarted = true;
        _clientp = nullptr;
    }
    void _flush() {
        uint8_t bytes[HS_FLUSH_BLOCK];
        while (_clientp && _clientp->available()) {
            delay(1);
            GHTTP_ESP_YIELD();
            _clientp->readBytes(bytes, min(_clientp->available(), HS_FLUSH_BLOCK));
        }
    }
};

}  // namespace ghttp