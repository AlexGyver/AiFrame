#include <Arduino.h>

#include "config.h"
#include "db.h"
#include "gen.h"
#include "settings.h"
#include "tft.h"

void setup() {
    Serial.begin(115200);
    Serial.println();

    db_init();
    sett_init();
    tft_init();

    // ======= AI =======
    gen.setKey(db[kk::kand_token], db[kk::kand_secret]);

    // ======= AP =======
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("Kandinsky AP");
    
    tft.print("AiFrame v");
    tft.println(F_VERSION);
    tft.println("Kandinsky AP");
    tft.print("IP: ");
    tft.println(WiFi.softAPIP());
    tft.println();

    // ======= STA =======
    bool wifi_ok = false;

    if (db[kk::wifi_ssid].length()) {
        WiFi.begin(db[kk::wifi_ssid], db[kk::wifi_pass]);
        wifi_ok = true;
        tft.print("Connecting");
        int tries = 20;
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            tft.print('.');
            if (!--tries) {
                wifi_ok = false;
                break;
            }
        }
        tft.println();
        tft.print("IP: ");
        tft.println(WiFi.localIP());
    } else {
        tft.println("STA not configured");
    }
    tft.println();

    if (!wifi_ok) return;

    // ======= STYLES =======
    tft.println("Gettings styles...");
    if (gen.getStyles()) {
        tft.print("OK. Styles: ");
        tft.println(gen.styles);
    } else {
        tft.print("Error! ");
        tft.println(gen.styles);
    }
    tft.println();

    // ======= MODEL =======
    tft.println("Init AI model...");
    if (gen.begin()) {
        tft.print("OK. Model: ");
        tft.println(gen.modelID());
    } else {
        tft.println("Error!");
    }
    tft.println();

    tft.println("Ready!");
    Serial.println("Ready!");

    if (WiFi.status() == WL_CONNECTED) ota.checkUpdate();
}

void loop() {
    sett_tick();
    gen_tick();
}