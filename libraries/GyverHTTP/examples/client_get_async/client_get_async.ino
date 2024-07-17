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

#if defined(ESP8266)
BearSSL::WiFiClientSecure client;
#elif defined(ESP32)
WiFiClientSecure client;
#endif

ghttp::Client http(client, "raw.githubusercontent.com", 443);

void setup() {
    Serial.begin(115200);

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected");
    Serial.println(WiFi.localIP());

    client.setInsecure();

    http.request("/GyverLibs/GyverHub-example/main/project.json");
}

// custom header collector
class Collector : public ghttp::HeadersCollector {
   public:
    void header(Text& name, Text& value) {
        Serial.print(name);
        Serial.print(": ");
        Serial.println(value);
    }
};

void loop() {
    if (http.available()) {
        Collector collector;
        ghttp::Client::Response resp = http.getResponse(&collector);
        if (resp) {
            Serial.println(resp.type());
            Serial.println(resp.body().length());
            Serial.println(resp.body());
        } else {
            Serial.println("response error");
        }
    }
}