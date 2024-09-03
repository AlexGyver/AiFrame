#include <Arduino.h>
#include <AutoOTA.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

#define WIFI_SSID ""
#define WIFI_PASS ""

AutoOTA ota("1.0", "GyverLibs/GyverHub-example/main/project.json");

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.print("Version ");
    Serial.println(ota.version());

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected");
    Serial.println(WiFi.localIP());

    String ver, notes;
    if (ota.checkUpdate(&ver, &notes)) {
        Serial.println(ver);
        Serial.println(notes);
        ota.update();
    }

    // AutoOTA ota("1.0", "GyverLibs/GyverHub-example/main/project.json");
    // if (ota.checkUpdate()) {
    //     ota.update();
    // }
}

void loop() {
    if (ota.tick()) {
        Serial.println((int)ota.getError());
    }
}