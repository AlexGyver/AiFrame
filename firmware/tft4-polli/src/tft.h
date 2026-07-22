#pragma once
#include <Adafruit_GFX.h>

#include "Adafruit_ST7796S_kbv.h"
#include "gen.h"

// SCL->D5
// SDA->D7
// BLK->3V3
#define TFT_CS D3
#define TFT_DC D8
#define TFT_RST D6

Adafruit_ST7796S_kbv tft(TFT_CS, TFT_DC, TFT_RST);

void tft_render(int x, int y, int w, int h, uint8_t* buf) {
    tft.drawRGBBitmap(x, y, (uint16_t*)buf, w, h);
}

void tft_init() {
    SPI.setFrequency(4000000ul);
    tft.begin();
    tft.setRotation(0);
    tft.fillScreen(0x0000);
    tft.setTextColor(0x07E0);
    tft.setTextSize(2, 2);
    gen.onRender(tft_render);
}