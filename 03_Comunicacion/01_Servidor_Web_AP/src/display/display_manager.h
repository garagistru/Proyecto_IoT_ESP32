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

    // Для предотвращения мерцания (перерисовываем только при изменении)
    int prevTotal;
    int prevActivos;
    int prevDormidos;
    unsigned long lastUpdateTimeSec;

    // Внутренние функции
    void drawStaticUI();
    void drawSpider(uint16_t x, uint16_t y, uint16_t color);
    void drawRightAlignedNumber(int16_t x_right, int16_t y, int value, uint16_t color);
    String formatTime(unsigned long totalSeconds);

public:
    DisplayManager();
    void init();

    // Публичные методы обновления
    void setNetworkStatus(bool connected);
    void updateMetrics(int total, int activos, int dormidos);
    void updateTime(unsigned long secondsSinceMidnight); // Или просто секунды для демо
};

extern DisplayManager display;

#endif // DISPLAY_MANAGER_H