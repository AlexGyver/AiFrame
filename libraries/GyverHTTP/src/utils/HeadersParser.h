#pragma once
#include <Arduino.h>
#include <Client.h>
#include <StringUtils.h>

// #define GHTTP_HEADERS_LOG Serial
#define GHTTP_HEADERS_TIMEOUT 500

#include "utils/cfg.h"

namespace ghttp {

class HeadersCollector {
   public:
    virtual void header(Text& name, Text& value) = 0;
};

class HeadersParser {
   public:
    HeadersParser(::Client& client, size_t bufsize = 150, HeadersCollector* collector = nullptr) {
        contentType.reserve(50);
        bool connF = 0, typeF = 0, lengthF = 0, encodingF = 0;
        uint8_t* buf = new uint8_t[bufsize];
        if (!buf) return;

        while (client.connected()) {
            GHTTP_ESP_YIELD();
            size_t n = client.readBytesUntil('\n', buf, bufsize);

            if (n == bufsize && buf[n - 1] != '\n') {
                uint32_t tmr = millis();
                while (client.read() != '\n') {
                    GHTTP_ESP_YIELD();
                    if (millis() - tmr >= GHTTP_HEADERS_TIMEOUT) break;
                }
                continue;
            }

            if (!n || buf[n - 1] != '\r') break;  // пустая или не оканчивается на \r
            if (n == 1) {                         // == \r
                valid = true;
                break;
            }

            Text header(buf, n - 1);

#ifdef GHTTP_HEADERS_LOG
            GHTTP_HEADERS_LOG.println(header);
#endif

            int16_t colon = header.indexOf(':');
            if (colon > 0) {
                Text name = header.substring(0, colon);
                Text value = header.substring(colon + 2);  // ": "
                if (collector) collector->header(name, value);
                if (!typeF && name == F("Content-Type")) {
                    value.addString(contentType);
                    typeF = 1;
                } else if (!lengthF && name == F("Content-Length")) {
                    length = value.toInt32();
                    lengthF = 1;
                } else if (!encodingF && name == F("Transfer-Encoding")) {
                    chunked = (value == F("chunked"));
                    encodingF = 1;
                } else if (!connF && name == F("Connection")) {
                    close = (value == F("close"));
                    connF = 1;
                }
            }
        }
        delete[] buf;
    }

    String contentType;
    size_t length = 0;
    bool close = false;
    bool valid = false;
    bool chunked = false;

    operator bool() {
        return valid;
    }
};

}  // namespace ghttp