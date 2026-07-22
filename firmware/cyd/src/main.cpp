#include <Arduino.h>

#include "config.h"
#include "db.h"
#include "gen.h"
#include "settings.h"
#include "tft.h"

void show_splash() {
    tft.fillScreen(TFT_BLACK);

    // Title — centered, large
    tft.setTextSize(3);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    const char* title = "AI Picture Frame";
    int titleX = (tft.width() - tft.textWidth(title)) / 2;
    tft.setCursor(titleX < 0 ? 0 : titleX, 10);
    tft.print(title);

    tft.drawFastHLine(0, 52, tft.width(), TFT_DARKGREY);

    // IP and SSID
    tft.setTextSize(2);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);

    String ip = (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
    tft.setCursor(8, 62);
    tft.print("IP:   "); tft.print(ip);

    String ssid = db[kk::wifi_ssid].toString();
    tft.setCursor(8, 88);
    tft.print("SSID: "); tft.print(ssid.length() ? ssid : String("-"));

    tft.drawFastHLine(0, 118, tft.width(), TFT_DARKGREY);

    // Prompt
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setCursor(8, 128);
    String query = db[kk::gen_query].toString();
    tft.print(query.length() ? query : String("(not set)"));
}

void setup() {
    Serial.begin(115200);
    Serial.println();

    // RGB LED off (active LOW)
    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);
    led_idle();

    WiFi.mode(WIFI_AP_STA);

    db_init();
    sett_init();
    tft_init();

    gen.setKey(db[kk::poll_key]);
    gen.setFS(LittleFS);

    // ======= AP =======
    WiFi.softAP("AiFrame CYD");

    // ======= STA =======
    bool wifi_ok = false;

    if (db[kk::wifi_ssid].length()) {
        tft.setTextSize(2);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.setCursor(0, 0);
        tft.print("Connecting");
        WiFi.begin(db[kk::wifi_ssid], db[kk::wifi_pass]);
        wifi_ok = true;
        int tries = 20;
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            tft.print('.');
            if (!--tries) {
                wifi_ok = false;
                break;
            }
        }
    }

    show_splash();

    if (!wifi_ok) return;

    Serial.println("Ready!");
    if (WiFi.status() == WL_CONNECTED) ota.checkUpdate();
}

void loop() {
    sett_tick();
    touch_tick();
    gen_tick();
    led_tick();
}
