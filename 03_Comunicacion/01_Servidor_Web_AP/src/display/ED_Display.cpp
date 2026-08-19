// src/display/ED_Display.cpp
#include "ED_Display.h"
#include "ED_State.h"

extern DisplayState sysState;

ED_Display::ED_Display()
    : tft(TFT_CS, TFT_DC, TFT_RST)
{
    _colorAccent = 0xEC38; // Фиолетовый
    _colorText = 0xFFFF;   // Белый
    _colorBg = 0x0000;     // Черный
}

void ED_Display::begin()
{
    // --- 1. SPI ---
    SPI.begin(12, -1, 11, 10);

    // --- 2. Подсветка ---
    ledcSetup(0, 5000, 8);
    ledcAttachPin(TFT_BL, 0);
    ledcWrite(0, 255);

    // --- 3. Аппаратный сброс ---
    pinMode(TFT_RST, OUTPUT);
    digitalWrite(TFT_RST, HIGH);
    delay(10);
    digitalWrite(TFT_RST, LOW);
    delay(10);
    digitalWrite(TFT_RST, HIGH);
    delay(150);

    // --- 4. Инициализация ---
    tft.init(240, 320);
    tft.setRotation(0);

    // --- 5. Очистка ---
    clear();
    drawHeader();
    drawRealTimeData();
}

void ED_Display::clear()
{
    tft.fillScreen(_colorBg);
}

void ED_Display::drawHeader()
{
    // Фон шапки
    uint16_t headerBg = 0x1082;
    tft.fillRect(0, 0, 240, 28, headerBg);

    // Нижняя граница
    uint16_t border = 0x5208;
    tft.drawLine(0, 27, 240, 27, border);

    // Название проекта
    tft.setTextColor(_colorAccent);
    tft.setTextSize(2);
    tft.setCursor(10, 8);
    tft.print("EnrollaDatos");

    // Версия
    uint16_t versionColor = 0x6B6B;
    tft.setTextColor(versionColor);
    tft.setTextSize(1);
    tft.setCursor(170, 8);
    tft.print("v1.2.0");

    // Тонкая линия под шапкой
    tft.drawLine(0, 30, 240, 30, 0x2108);
}

// ============================================
// ОТРИСОВКА ТОЛЬКО СТАТУСНОЙ ИНФОРМАЦИИ
// ============================================
void ED_Display::drawRealTimeData()
{
    // --- Очищаем область данных (ниже шапки) ---
    tft.fillRect(0, 35, 240, 260, COLOR_BG);

    // --- 1. СТАТУС СЕТИ ---
    tft.setTextColor(COLOR_YELLOW, COLOR_BG);
    tft.setTextSize(2);
    tft.setCursor(10, 45);
    tft.print("RED");

    tft.setTextColor(sysState.isConnected ? COLOR_GREEN : COLOR_RED, COLOR_BG);
    tft.setCursor(70, 49);
    tft.print(sysState.isConnected ? "Conectado" : "Desconectado");

    // --- 2. КОЛИЧЕСТВО УЗЛОВ ---
    tft.setTextColor(COLOR_WHITE, COLOR_BG);
    tft.setTextSize(2);
    tft.setCursor(10, 85);
    tft.print("Total:");
    tft.setCursor(120, 85);
    tft.print(sysState.totalNodes);

    tft.setCursor(10, 115);
    tft.print("Activos:");
    tft.setCursor(120, 115);
    tft.setTextColor(COLOR_GREEN, COLOR_BG);
    tft.print(sysState.activeNodes);

    tft.setTextColor(COLOR_WHITE, COLOR_BG);
    tft.setCursor(10, 145);
    tft.print("Dormidos:");
    tft.setCursor(120, 145);
    tft.setTextColor(COLOR_GRAY_LIGHT, COLOR_BG);
    tft.print(sysState.dormantNodes);

    // --- 3. ВРЕМЯ ПЕРЕДАЧИ ---
    tft.setTextColor(COLOR_WHITE, COLOR_BG);
    tft.setTextSize(2);
    tft.setCursor(10, 185);
    tft.print("Recepción:");
    tft.setCursor(140, 185);
    tft.setTextColor(COLOR_CYAN, COLOR_BG);
    tft.print(sysState.lastReceive);

    tft.setTextColor(COLOR_WHITE, COLOR_BG);
    tft.setCursor(10, 215);
    tft.print("Envío:");
    tft.setCursor(140, 215);
    tft.setTextColor(COLOR_CYAN, COLOR_BG);
    tft.print(sysState.lastTransmit);

    // --- 4. БУФЕР ---
    tft.setTextColor(COLOR_WHITE, COLOR_BG);
    tft.setCursor(10, 245);
    tft.print("Buffer:");
    tft.setCursor(140, 245);
    tft.setTextColor(COLOR_YELLOW, COLOR_BG);
    tft.print(sysState.bufferSize);

    // --- 5. Разделительная линия перед подвалом ---
    tft.drawLine(0, 275, 240, 275, COLOR_GRAY_DARK);
}

void ED_Display::setBrightness(uint8_t level)
{
    ledcWrite(0, level);
}