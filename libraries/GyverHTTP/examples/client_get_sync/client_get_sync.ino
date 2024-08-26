#include <Arduino.h>
#include <GyverHTTP.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureBearSSL.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <WiFiClientSecure.h>
#endif

#define WIFI_SSID ""
#define WIFI_PASS ""

void setup() {
    Serial.begin(115200);

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected");
    Serial.println(WiFi.localIP());

#if defined(ESP8266)
    BearSSL::WiFiClientSecure client;
#elif defined(ESP32)
    WiFiClientSecure client;
#endif

    client.setInsecure();

    ghttp::Client http(client, "raw.githubusercontent.com", 443);

    if (http.request("/GyverLibs/GyverHub-example/main/project.json")) {
        ghttp::Client::Response resp = http.getResponse();
        if (resp) {
            Serial.println(resp.code());
            Serial.println(resp.type());
            Serial.println(resp.body().length());
            Serial.println(resp.body());

            // парсинг json body при помощи GSON
            // работает также с chunked encoding
            // String s = resp.body().readString();
            // gson::Parser json;
            // if (json.parse(s)) {
            //     json.stringify(Serial);
            // } else {
            //     Serial.println("Parse error");
            // }
        } else {
            Serial.println("response error");
        }
    } else {
        Serial.println("connect error");
    }
}
void loop() {
}