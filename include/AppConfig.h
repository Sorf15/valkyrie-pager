#pragma once

// Remote API
constexpr char API_BASE_URL[] = "https://example.com";

// Hardware pins
#define TFT_CS 10
#define TFT_DC 9
#define TFT_RST 8
#define TFT_MOSI 7
#define TFT_SCLK 6
#define BTN_ENTER_BACK 20
#define BTN_DOWN 0
#define BTN_UP 21
#define BAT_PIN 1

// Display layout and colours
#define TFT_WIDTH 320
#define TFT_HEIGHT 240
#define BAR_HEIGHT 25

#define COLOR_BG 0x0000
#define COLOR_BAR 0x001F
#define COLOR_TEXT 0xFFFF
#define COLOR_SELECT 0xFFFF
#define COLOR_SEL_TXT 0x0000
#define COLOR_RED 0xF800

#define MAX_HASH_BLOCKS 192
#define TOTAL_SETTING 4
