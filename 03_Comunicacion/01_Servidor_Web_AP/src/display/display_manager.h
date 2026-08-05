#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include "display_config.h"

class DisplayManager
{
private:
    Adafruit_ST7789 *tft;
    bool isInitialized;
    bool networkConnected;

    int prevTotal;
    int prevActivos;
    int prevDormidos;
    unsigned long lastDataTimeSec; // Время последней передачи данных
    unsigned long currentTimeSec;  // Текущее время работы системы

    void drawStaticUI();
    void drawRightAlignedNumber(int16_t x_right, int16_t y, int value, uint8_t textSize, uint16_t color);
    String formatTime(unsigned long totalSeconds);

public:
    DisplayManager();
    void init();
    void setNetworkStatus(bool connected);
    void updateMetrics(int total, int activos, int dormidos);
    void updateTime(unsigned long lastDataSeconds, unsigned long currentSeconds); // Два параметра!
};

extern DisplayManager display;

#endif // DISPLAY_MANAGER_H