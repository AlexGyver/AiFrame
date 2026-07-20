#pragma once

#define F_VERSION "1.0"

#ifdef HW_CYD40
  // 4.0" ST7796, 320×480 native portrait; rotation=1 → landscape 480×320
  #define DISP_WIDTH  480
  #define DISP_HEIGHT 320
  #define DISP_SCALE  1    // 1, 2, 4, 8
  #define TFT_BL      27   // backlight — verify for your board
#else
  // 2.8" ILI9341, landscape 320×240
  #define DISP_WIDTH  320
  #define DISP_HEIGHT 240
  #define DISP_SCALE  1    // 1, 2, 4, 8
  #define TFT_BL      21   // backlight (active HIGH)
#endif

// CYD RGB LED (active LOW) — same on both boards
#define LED_R 4
#define LED_G 16
#define LED_B 17
