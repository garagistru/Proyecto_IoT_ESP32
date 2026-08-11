// src/display/ED_Display.h
#ifndef ED_DISPLAY_H
#define ED_DISPLAY_H

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// ============================================
// ПИНЫ ДЛЯ ESP32-S3 (ПРОВЕРЕНО ДЛЯ 240x320)
// ============================================
#define TFT_MOSI 11 // Data Out
#define TFT_SCK 12  // Clock
#define TFT_CS 10   // Chip Select
#define TFT_DC 9    // Data/Command
#define TFT_RST 8   // Reset
#define TFT_BL 48   // Backlight (PWM)

// ============================================
// КЛАСС ДЛЯ УПРАВЛЕНИЯ ДИСПЛЕЕМ
// ============================================
class ED_Display
{
public:
    ED_Display();
    void begin();
    void drawHeader();
    void clear();
    void setBrightness(uint8_t level);
    void drawTestScreen();

private:
    Adafruit_ST7789 tft;
    uint16_t _colorAccent;
    uint16_t _colorText;
    uint16_t _colorBg;
};

#endif