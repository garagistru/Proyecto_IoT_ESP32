// src/display/ED_Display.h
#ifndef ED_DISPLAY_H
#define ED_DISPLAY_H

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// ============================================
// ПИНЫ
// ============================================
#define TFT_CS 10
#define TFT_RST 8
#define TFT_DC 9
#define TFT_BL 48

// ============================================
// ЦВЕТА (RGB565)
// ============================================
#define COLOR_BG 0x0000         // Черный
#define COLOR_WHITE 0xFFFF      // Белый
#define COLOR_YELLOW 0xFFE0     // Желтый
#define COLOR_GREEN 0x07E0      // Зеленый
#define COLOR_RED 0xF800        // Красный
#define COLOR_CYAN 0x07FF       // Голубой
#define COLOR_GRAY_LIGHT 0xAAAA // Светло-серый
#define COLOR_GRAY_DARK 0x5555  // Темно-серый

// ============================================
// КЛАСС
// ============================================
class ED_Display
{
public:
    ED_Display();
    void begin();
    void drawHeader();
    void drawRealTimeData(); // ← НОВЫЙ МЕТОД
    void clear();
    void setBrightness(uint8_t level);

private:
    Adafruit_ST7789 tft;
    uint16_t _colorAccent;
    uint16_t _colorText;
    uint16_t _colorBg;
};

#endif