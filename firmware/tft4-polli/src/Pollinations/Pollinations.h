#pragma once
#include <Arduino.h>
#include <FS.h>

#include "tjpgd/tjpgd.h"

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
extern "C" {
#include "user_interface.h"
}
#define POLL_CLIENT BearSSL::WiFiClientSecure
#else
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#define POLL_CLIENT WiFiClientSecure
#endif

// Same host/API as firmware/cyd
#define POLL_HOST       "gen.pollinations.ai"
// Pollinations returns progressive JPEG; tjpgd cannot decode it.
// wsrv.nl re-encodes to baseline JPEG while fetching the origin.
#define POLL_PROXY_HOST "wsrv.nl"
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
        _api_key.trim();
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
        if (!_fs) { status = "no fs"; return false; }
        status = "generating";

        // Origin URL — same format as firmware/cyd
        String origin = "https://";
        origin += POLL_HOST;
        origin += "/image/";
        origin += urlEncode(query);
        origin += "?model=flux&width=";
        origin += width;
        origin += "&height=";
        origin += height;
        if (negative.length()) { origin += "&negative="; origin += urlEncode(negative); }
        if (_api_key.length()) { origin += "&key="; origin += _api_key; }
        origin += "&seed=";
        origin += (uint32_t)millis();
        origin += "&nologo=true";

        // Proxy re-encodes progressive → baseline JPEG for tjpgd
        String url = "https://";
        url += POLL_PROXY_HOST;
        url += "/?url=";
        url += urlEncode(origin);
        url += "&output=jpg&q=85&w=";
        url += width;
        url += "&h=";
        url += height;
        url += "&fit=cover";

        POLL_LOG("GET proxy " + String(POLL_PROXY_HOST));
        POLL_LOG("origin " + _redactKey(origin));
        POLL_LOG(String("Free heap: ") + ESP.getFreeHeap());

#ifdef ESP8266
        wifi_set_sleep_type(NONE_SLEEP_T);
#endif

        POLL_CLIENT client;
#ifdef ESP8266
        _setupBearSSL(client, POLL_PROXY_HOST);
#endif
        client.setInsecure();

        HTTPClient http;
#ifdef ESP8266
        http.useHTTP10(true);
        http.setReuse(false);
        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
#endif
        // Proxy waits for Pollinations generation + re-encode
        http.setTimeout(90000);

        if (!http.begin(client, url)) {
            POLL_LOG("http.begin failed");
            status = "begin fail";
            return false;
        }

        http.addHeader("User-Agent", "AiFrame/1.0");
        http.addHeader("Accept", "image/jpeg");
        http.addHeader("Connection", "close");

        POLL_LOG(String("heap before GET: ") + ESP.getFreeHeap());
        int code = http.GET();
        Serial.print("HTTP code: ");
        Serial.println(code);
#ifdef ESP8266
        if (code < 0) {
            POLL_LOG(http.errorToString(code));
        }
#endif

        if (code != HTTP_CODE_OK) {
            String body = http.getString();
            POLL_LOG("Error: " + body);
            http.end();
            status = "http " + String(code);
            return false;
        }

        POLL_LOG(String("Content-Length: ") + http.getSize());

        // Save via writeToStream so any remaining encoding is handled
        bool saved = false;
        int written = 0;
        File tmpFile = _fs->open(POLL_IMAGE_TMP, "w");
        if (tmpFile) {
            written = http.writeToStream(&tmpFile);
            tmpFile.flush();
            tmpFile.close();
            saved = written > 0;
            POLL_LOG(String("saved bytes: ") + written);
        } else {
            POLL_LOG("save file open error");
        }
        http.end();

        if (!saved) { status = "save error"; return false; }

        if (!_jpegLooksValid(written)) {
            _fs->remove(POLL_IMAGE_TMP);
            return false;
        }

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

#ifdef ESP8266
    void _setupBearSSL(POLL_CLIENT& client, const char* host) {
        const int sizes[] = {2048, 1024, 512};
        bool ok = false;
        for (int s : sizes) {
            if (client.probeMaxFragmentLength(host, 443, s)) {
                client.setBufferSizes(s, s);
                POLL_LOG(String("MFLN ok, buffers=") + s);
                ok = true;
                break;
            }
        }
        if (!ok) {
            uint32_t heap = ESP.getFreeHeap();
            int rx = 4096;
            if (heap > 36000) rx = 8192;
            if (heap > 48000) rx = 16384;
            client.setBufferSizes(rx, 512);
            POLL_LOG(String("No MFLN, RX=") + rx + " heap=" + heap);
        }
    }
#endif

    static String _redactKey(const String& url) {
        int k = url.indexOf("key=");
        if (k < 0) return url;
        int end = url.indexOf('&', k);
        String out = url.substring(0, k + 4) + "***";
        if (end >= 0) out += url.substring(end);
        return out;
    }

    bool _jpegLooksValid(int expectedSize) {
        File f = _fs->open(POLL_IMAGE_TMP, "r");
        if (!f) {
            status = "open error";
            return false;
        }
        size_t sz = f.size();
        uint8_t head[16];
        memset(head, 0, sizeof(head));
        f.read(head, sizeof(head));

        // check EOI near end
        uint8_t tail[2] = {0, 0};
        if (sz >= 2) {
            f.seek(sz - 2, SeekSet);
            f.read(tail, 2);
        }
        f.close();

        String hex;
        for (int i = 0; i < 8; i++) {
            if (i) hex += ' ';
            if (head[i] < 16) hex += '0';
            hex += String(head[i], HEX);
        }
        POLL_LOG(String("file size=") + sz + " head=" + hex +
                 " tail=" + String(tail[0], HEX) + " " + String(tail[1], HEX));

        if (head[0] == 0x89 && head[1] == 0x50) {
            status = "png unsupported";
            POLL_LOG("got PNG, tjpgd cannot decode it");
            return false;
        }
        if (head[0] != 0xFF || head[1] != 0xD8) {
            status = "not jpeg";
            POLL_LOG("not a JPEG file");
            return false;
        }
        if (tail[0] != 0xFF || tail[1] != 0xD9) {
            POLL_LOG("WARNING: missing JPEG EOI — file may be truncated/corrupt");
        }

        // Scan for progressive SOF2 (0xFFC2) — not supported by tjpgd
        File s = _fs->open(POLL_IMAGE_TMP, "r");
        if (s) {
            uint8_t prev = 0;
            bool progressive = false;
            while (s.available()) {
                uint8_t b = s.read();
                if (prev == 0xFF && b == 0xC2) { progressive = true; break; }
                if (prev == 0xFF && (b == 0xC0 || b == 0xC1)) break;  // baseline
                prev = b;
                yield();
            }
            s.close();
            if (progressive) {
                status = "prog jpeg";
                POLL_LOG("progressive JPEG — not supported by tjpgd");
                return false;
            }
        }
        return true;
    }

    static size_t jd_input_cb(JDEC* jdec, uint8_t* buf, size_t len) {
        yield();
        if (!self || !self->_jpegStream) return 0;
        if (buf) {
            return self->_jpegStream->readBytes(buf, len);
        }
        // skip bytes — must return actual skipped count
        uint8_t tmp[64];
        size_t got = 0;
        while (got < len) {
            size_t chunk = min(len - got, (size_t)sizeof(tmp));
            size_t r = self->_jpegStream->readBytes(tmp, chunk);
            got += r;
            if (r == 0) break;
        }
        return got;
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

    static const char* _jresName(JRESULT res) {
        switch (res) {
            case JDR_OK: return "OK";
            case JDR_INTR: return "INTR";
            case JDR_INP: return "INP";
            case JDR_MEM1: return "MEM1";
            case JDR_MEM2: return "MEM2";
            case JDR_PAR: return "PAR";
            case JDR_FMT1: return "FMT1(broken)";
            case JDR_FMT2: return "FMT2(unsupported)";
            case JDR_FMT3: return "FMT3(not jpeg std)";
            default: return "?";
        }
    }

    bool decodeJpeg(Stream& stream) {
        uint8_t* workspace = new uint8_t[TJPGD_WORKSPACE_SIZE];
        if (!workspace) { POLL_LOG("alloc error"); return false; }

        JDEC jdec;
        // Adafruit_GFX drawRGBBitmap = native RGB565 (cyd uses swap=1 for TFT_eSPI)
        jdec.swap = 0;
        _jpegStream = &stream;
        self = this;

        JRESULT res = jd_prepare(&jdec, jd_input_cb, workspace, TJPGD_WORKSPACE_SIZE, nullptr);
        if (res == JDR_OK) {
            POLL_LOG(String("jpeg ") + jdec.width + "x" + jdec.height);
            res = jd_decomp(&jdec, jd_output_cb, _scale);
            if (res == JDR_OK && _end_cb) _end_cb();
        }

        self = nullptr;
        _jpegStream = nullptr;
        delete[] workspace;

        if (res != JDR_OK) POLL_LOG(String("tjpgd error: ") + (int)res + " " + _jresName(res));
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
