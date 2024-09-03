#pragma once
#include <Arduino.h>

// config
#define FUSION_HOST "api-key.fusionbrain.ai"
#define FUSION_PORT 443
#define FUSION_PERIOD 6000
#define FUSION_TRIES 5
#define FUS_LOG(x) Serial.println(x)

// #define GHTTP_HEADERS_LOG Serial
#include <GSON.h>
#include <GyverHTTP.h>

#include "StreamB64.h"
#include "tjpgd/tjpgd.h"

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureBearSSL.h>
#define FUSION_CLIENT BearSSL::WiFiClientSecure
#else
#include <WiFi.h>
#include <WiFiClientSecure.h>
#define FUSION_CLIENT WiFiClientSecure
#endif

class Kandinsky {
    typedef std::function<void(int x, int y, int w, int h, uint8_t* buf)> RenderCallback;
    typedef std::function<void()> RenderEndCallback;

    enum class State : uint8_t {
        GetModels,
        Generate,
        Status,
        GetStyles,
    };

   public:
    Kandinsky() {}
    Kandinsky(const String& apikey, const String& secret_key) {
        setKey(apikey, secret_key);
    }

    void setKey(const String& apikey, const String& secret_key) {
        if (apikey.length() && secret_key.length()) {
            _api_key = "Key " + apikey;
            _secret_key = "Secret " + secret_key;
        }
    }

    void onRender(RenderCallback cb) {
        _rnd_cb = cb;
    }

    void onRenderEnd(RenderEndCallback cb) {
        _end_cb = cb;
    }

    // 1, 2, 4, 8
    void setScale(uint8_t scale) {
        switch (scale) {
            case 1:
                _scale = 0;
                break;
            case 2:
                _scale = 1;
                break;
            case 4:
                _scale = 2;
                break;
            case 8:
                _scale = 3;
                break;
            default:
                _scale = 0;
                break;
        }
    }

    bool begin() {
        if (!_api_key.length()) return 0;
        return request(State::GetModels, FUSION_HOST, "/key/api/v1/models");
    }

    bool getStyles() {
        if (!_api_key.length()) return 0;
        return request(State::GetStyles, "cdn.fusionbrain.ai", "/static/styles/key");
    }

    bool generate(Text query, uint16_t width = 512, uint16_t height = 512, Text style = "DEFAULT", Text negative = "") {
        status = "wrong config";
        if (!_api_key.length()) return 0;
        if (!style.length()) return 0;
        if (!query.length()) return 0;
        if (_id < 0) return 0;

        gson::string json;
        json.beginObj();
        json.addString(F("type"), F("GENERATE"));
        json.addString(F("style"), style);
        json.addString(F("negativePromptUnclip"), negative);
        json.addInt(F("width"), width);
        json.addInt(F("height"), height);
        json.addInt(F("num_images"), 1);
        {
            json.beginObj(F("generateParams"));
            json.addString(F("query"), query);
            json.endObj();
        }
        json.endObj(true);

        ghttp::Client::FormData data;
        data.add("model_id", "", "", Value(_id));
        data.add("params", "blob", "application/json", json);

        uint8_t tries = FUSION_TRIES;
        while (tries--) {
            if (request(State::Generate, FUSION_HOST, "/key/api/v1/text2image/run", "POST", &data)) {
                FUS_LOG("Gen request sent");
                status = "wait result";
                return 1;
            } else {
                FUS_LOG("Gen request error");
                delay(2000);
            }
        }
        status = "gen request error";
        return 0;
    }

    bool getImage() {
        if (!_api_key.length()) return 0;
        if (!_uuid.length()) return 0;
        FUS_LOG("Check status...");
        String url("/key/api/v1/text2image/status/");
        url += _uuid;
        return request(State::Status, FUSION_HOST, url);
    }

    void tick() {
        if (_uuid.length() && millis() - _tmr >= FUSION_PERIOD) {
            _tmr = millis();
            getImage();
        }
    }

    int modelID() {
        return _id;
    }

    String styles = "DEFAULT;ANIME;UHD;KANDINSKY";
    String status = "idle";

   private:
    String _api_key;
    String _secret_key;
    String _uuid;
    uint8_t _scale = 0;
    uint32_t _tmr = 0;
    int _id = -1;
    RenderCallback _rnd_cb = nullptr;
    RenderEndCallback _end_cb = nullptr;
    StreamB64* _stream = nullptr;

    // static
    static Kandinsky* self;

    static size_t jd_input_cb(JDEC* jdec, uint8_t* buf, size_t len) {
        if (self) {
            self->_stream->readBytes(buf, len);
        }
        return len;
    }

    static int jd_output_cb(JDEC* jdec, void* bitmap, JRECT* rect) {
        if (self && self->_rnd_cb) {
            self->_rnd_cb(rect->left, rect->top, rect->right - rect->left + 1, rect->bottom - rect->top + 1, (uint8_t*)bitmap);
        }
        return 1;
    }

    // system
    bool request(State state, Text host, Text url, Text method = "GET", ghttp::Client::FormData* data = nullptr) {
        FUSION_CLIENT client;
#ifdef ESP8266
        client.setBufferSizes(512, 512);
#endif
        client.setInsecure();
        ghttp::Client http(client, host.str(), FUSION_PORT);

        ghttp::Client::Headers headers;
        headers.add("X-Key", _api_key);
        headers.add("X-Secret", _secret_key);

        bool ok;
        if (data) {
            ok = http.request(url, method, headers, *data);
        } else {
            ok = http.request(url, method, headers);
        }
        if (!ok) {
            FUS_LOG("Request error");
            return 0;
        }

        ghttp::Client::Response resp = http.getResponse();
        if (resp && resp.code() >= 200 && resp.code() < 300) {
            if (state == State::Status) {
                bool ok = parseStatus(resp.body());
                http.flush();
                return ok;
            } else {
                gtl::stack_uniq<uint8_t> str;
                resp.body().writeTo(str);
                gson::Parser json;

                if (json.parse(str.buf(), str.length())) {
                    return parse(state, json);
                } else {
                    FUS_LOG("Parse error");
                }
            }
        } else {
            http.flush();
            FUS_LOG("Response error");
        }
        return 0;
    }

    bool parseStatus(Stream& stream) {
        bool found = 0;
        while (stream.available()) {
            stream.readStringUntil('\"');
            String key = stream.readStringUntil('\"');
            if (key == "images") {
                found = 1;
                break;
            }
            stream.readStringUntil('\"');
            String val = stream.readStringUntil('\"');
            if (!key.length() || !val.length()) break;

            if (key == "status") {
                switch (Text(val).hash()) {
                    case SH("INITIAL"):
                    case SH("PROCESSING"):
                        return 1;
                    case SH("DONE"):
                        _uuid = "";
                        break;
                    case SH("FAIL"):
                        _uuid = "";
                        status = "gen fail";
                        return 0;
                }
            }
        }
        if (found) {
            stream.readStringUntil('\"');
            uint8_t* workspace = new uint8_t[TJPGD_WORKSPACE_SIZE];
            if (!workspace) {
                FUS_LOG("allocate error");
                return 0;
            }

            JDEC jdec;
            jdec.swap = 0;
            JRESULT jresult = JDR_OK;
            StreamB64 sb64(stream);
            _stream = &sb64;
            self = this;

            jresult = jd_prepare(&jdec, jd_input_cb, workspace, TJPGD_WORKSPACE_SIZE, 0);

            if (jresult == JDR_OK) {
                jresult = jd_decomp(&jdec, jd_output_cb, _scale);

                if (jresult == JDR_OK) {
                    if (_end_cb) _end_cb();
                } else {
                    FUS_LOG("jdec error");
                }
            } else {
                FUS_LOG("jdec error");
            }

            self = nullptr;
            delete[] workspace;

            status = jresult == JDR_OK ? "gen done" : "jpg error";
            return jresult == JDR_OK;
        }
        return 1;
    }

    bool parse(State state, gson::Parser& json) {
        switch (state) {
            case State::GetStyles:
                styles = "";
                for (int i = 0; i < (int)json.rootLength(); i++) {
                    if (i) styles += ';';
                    json[i]["name"].addString(styles);
                }
                if (styles.length()) return 1;
                break;

            case State::GetModels:
                _id = json[0]["id"];
                if (_id >= 0) return 1;
                break;

            case State::Generate:
                _tmr = millis();
                json["uuid"].toString(_uuid);
                if (_uuid.length()) return 1;
                break;

            default:
                break;
        }
        return 0;
    }
};

Kandinsky* Kandinsky::self __attribute__((weak)) = nullptr;
