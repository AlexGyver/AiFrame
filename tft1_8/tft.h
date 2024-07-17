#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#include "gen.h"

// SCL->D5
// SDA->D7
// BLK->3V3
#define TFT_CS D3
#define TFT_DC D8
#define TFT_RST D6

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

void tft_render(int x, int y, int w, int h, uint8_t* buf) {
    tft.drawRGBBitmap(x, y, (uint16_t*)buf, w, h);
}

void tft_init() {
    SPI.setFrequency(4000000ul);
    tft.initR(INITR_BLACKTAB);
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_GREEN);

    gen.onRender(tft_render);
}