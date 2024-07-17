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
#define BOT_TOKEN ""
#define CHAT_ID ""

#if defined(ESP8266)
BearSSL::WiFiClientSecure client;
#elif defined(ESP32)
WiFiClientSecure client;
#endif

ghttp::Client http(client, "api.telegram.org", 443);

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

    Text json = "{\"chat_id\":" CHAT_ID ",\"text\":\"hello!\"}";
    Text headers = "Content-Type: application/json\r\n";
    http.request("/bot" BOT_TOKEN "/sendMessage", "GET", headers, json);

    http.onResponse([](ghttp::Client::Response& resp) {
        Serial.println(resp.type());
        Serial.println(resp.body().length());
        Serial.println(resp.body());
    });
}
void loop() {
    http.tick();
}