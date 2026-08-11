// src/display/ED_Display.h
#ifndef ED_DISPLAY_H
#define ED_DISPLAY_H

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// ============================================
// ПИНЫ ДИСПЛЕЯ (ESP32-S3)
// ============================================
#define TFT_MOSI 11
#define TFT_SCK 12
#define TFT_CS 10
#define TFT_DC 9
#define TFT_RST 8
#define TFT_BL 48

// ============================================
// КЛАСС ДЛЯ УПРАВЛЕНИЯ ДИСПЛЕЕМ
// ============================================
class ED_Display
{
public:
    ED_Display();
    void begin();
    void drawHeader();
    void drawRealTimeData(); // <--- ДОБАВЛЕНО: Объявление функции реальных данных
    void clear();
    void setBrightness(uint8_t level);

private:
    Adafruit_ST7789 tft;
};

#endif