#pragma once
#include <TFT_eSPI.h>

#include "config.h"
#include "gen.h"

// CYD SPI pins configured via platformio.ini build_flags:
// TFT_CS=15, TFT_DC=2, TFT_RST=-1, TFT_MOSI=13, TFT_SCLK=14, TFT_MISO=12
// Backlight: TFT_BL (active HIGH), Touch: TOUCH_CS=33

TFT_eSPI tft;

void tft_render(int x, int y, int w, int h, uint8_t* buf) {
    tft.pushImage(x, y, w, h, (uint16_t*)buf);
}

void tft_draw_status(const char* msg) {
    int barH = 24;
    int y = tft.height() - barH;
    tft.fillRect(0, y, tft.width(), barH, TFT_BLACK);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(4, y + 4);
    tft.print(msg);
}

void touch_tick() {
    static uint32_t lastTouch = 0;
    uint16_t tx, ty;
    if (tft.getTouch(&tx, &ty)) {
        uint32_t now = millis();
        if (now - lastTouch > 500) {
            lastTouch = now;
            Serial.println("touch: generate");
            generate();
        }
    }
}

void tft_init() {
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(2);

    gen.onRender(tft_render);
    gen_on_start([]() { tft_draw_status("Generating..."); });
    gen_on_end([]() {
        if (gen.status != "done") tft_draw_status(gen.status.c_str());
    });
}
