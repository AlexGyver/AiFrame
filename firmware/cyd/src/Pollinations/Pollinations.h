#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <FS.h>

#include "tjpgd/tjpgd.h"

#define POLL_HOST       "gen.pollinations.ai"
#define POLL_IMAGE_PATH "/image.jpg"
#define POLL_IMAGE_TMP  "/image.tmp"
#define POLL_LOG(x)     Serial.println(x)

class Pollinations {
    typedef std::function<void(int x, int y, int w, int h, uint8_t* buf)> RenderCallback;
    typedef std::function<void()> RenderEndCallback;

   public:
    Pollinations() {}

    void setKey(const String& key) {
        _api_key = key;
    }

    void setFS(fs::FS& fs) {
        _fs = &fs;
    }

    void onRender(RenderCallback cb) {
        _rnd_cb = cb;
    }

    void onRenderEnd(RenderEndCallback cb) {
        _end_cb = cb;
    }

    // scale: 1, 2, 4, 8 — maps to tjpgd 0, 1, 2, 3
    void setScale(uint8_t scale) {
        switch (scale) {
            case 2: _scale = 1; break;
            case 4: _scale = 2; break;
            case 8: _scale = 3; break;
            default: _scale = 0; break;
        }
    }

    bool generate(const String& query, uint16_t width = 320, uint16_t height = 240, const String& negative = "") {
        if (!query.length()) { status = "empty query"; return false; }
        status = "generating";

        String url = "https://";
        url += POLL_HOST;
        url += "/image/";
        url += urlEncode(query);
        url += "?model=flux&width=";
        url += width;
        url += "&height=";
        url += height;
        if (negative.length()) { url += "&negative="; url += urlEncode(negative); }
        if (_api_key.length()) { url += "&key="; url += _api_key; }
        url += "&seed=";
        url += (uint32_t)millis();
        url += "&nologo=true";

        POLL_LOG("GET " + url);

        WiFiClientSecure client;
        client.setInsecure();

        HTTPClient http;
        http.begin(client, url);
        http.setTimeout(30000);  // ms
        http.addHeader("User-Agent", "AiFrame/1.0 ESP32");
        http.addHeader("Accept", "image/jpeg");
        http.addHeader("Connection", "close");

        int code = http.GET();
        Serial.print("HTTP code: "); Serial.println(code);

        if (code != HTTP_CODE_OK) {
            String body = http.getString();
            POLL_LOG("Error: " + body);
            http.end();
            status = "http " + String(code);
            return false;
        }

        // Save via writeToStream so chunked transfer encoding is decoded properly.
        // getStreamPtr() returns the raw transport layer and includes chunk-size
        // bytes (e.g. "5a2f\r\n") that corrupt the saved file.
        bool saved = false;
        if (_fs) {
            File tmpFile = _fs->open(POLL_IMAGE_TMP, "w");
            if (tmpFile) {
                http.writeToStream(&tmpFile);
                tmpFile.close();
                saved = true;
            } else {
                POLL_LOG("save file open error");
            }
        }
        http.end();

        if (!saved) { status = "save error"; return false; }

        // Decode JPEG from the clean saved file
        File f = _fs->open(POLL_IMAGE_TMP, "r");
        bool ok = decodeJpeg(f);
        f.close();

        if (ok) {
            _fs->remove(POLL_IMAGE_PATH);
            bool renamed = _fs->rename(POLL_IMAGE_TMP, POLL_IMAGE_PATH);
            POLL_LOG(renamed ? "image saved OK" : "image rename FAILED");
        } else {
            _fs->remove(POLL_IMAGE_TMP);
        }

        status = ok ? "done" : "jpeg error";
        return ok;
    }

    String status = "idle";

   private:
    String _api_key;
    uint8_t _scale = 0;
    RenderCallback _rnd_cb = nullptr;
    RenderEndCallback _end_cb = nullptr;
    Stream* _jpegStream = nullptr;
    fs::FS* _fs = nullptr;

    static Pollinations* self;

    static size_t jd_input_cb(JDEC* jdec, uint8_t* buf, size_t len) {
        yield();
        if (!self || !self->_jpegStream) return 0;
        if (buf) {
            return self->_jpegStream->readBytes(buf, len);
        } else {
            uint8_t tmp[64];
            size_t remaining = len;
            while (remaining > 0) {
                size_t chunk = min(remaining, (size_t)sizeof(tmp));
                size_t r = self->_jpegStream->readBytes(tmp, chunk);
                remaining -= r;
                if (r == 0) break;
            }
            return len;
        }
    }

    static int jd_output_cb(JDEC* jdec, void* bitmap, JRECT* rect) {
        if (self && self->_rnd_cb) {
            self->_rnd_cb(rect->left, rect->top,
                          rect->right - rect->left + 1,
                          rect->bottom - rect->top + 1,
                          (uint8_t*)bitmap);
        }
        return 1;
    }

    bool decodeJpeg(Stream& stream) {
        uint8_t* workspace = new uint8_t[TJPGD_WORKSPACE_SIZE];
        if (!workspace) { POLL_LOG("alloc error"); return false; }

        JDEC jdec;
        jdec.swap = 1;
        _jpegStream = &stream;
        self = this;

        JRESULT res = jd_prepare(&jdec, jd_input_cb, workspace, TJPGD_WORKSPACE_SIZE, nullptr);
        if (res == JDR_OK) {
            res = jd_decomp(&jdec, jd_output_cb, _scale);
            if (res == JDR_OK && _end_cb) _end_cb();
        }

        self = nullptr;
        _jpegStream = nullptr;
        delete[] workspace;

        if (res != JDR_OK) POLL_LOG("tjpgd error");
        return res == JDR_OK;
    }

    static String urlEncode(const String& s) {
        String out;
        out.reserve(s.length() * 3);
        for (size_t i = 0; i < s.length(); i++) {
            char c = s[i];
            if (isAlphaNumeric(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                out += c;
            } else {
                char hex[4];
                snprintf(hex, sizeof(hex), "%%%02X", (uint8_t)c);
                out += hex;
            }
        }
        return out;
    }
};

Pollinations* Pollinations::self __attribute__((weak)) = nullptr;
