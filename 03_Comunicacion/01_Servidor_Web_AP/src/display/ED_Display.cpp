// src/display/ED_Display.cpp
#include "ED_Display.h"
#include "ED_State.h"
#include <WiFi.h>

extern DisplayState sysState;

ED_Display::ED_Display()
    : tft(TFT_CS, TFT_DC, TFT_RST)
{
    _colorAccent = COLOR_ACCENT;
    _colorText = COLOR_WHITE;
    _colorBg = COLOR_BG;
}

void ED_Display::begin()
{
    Serial.println("🔧 Инициализация дисплея...");

    SPI.begin(TFT_SCK, -1, TFT_MOSI, TFT_CS);
    ledcSetup(0, 5000, 8);
    ledcAttachPin(TFT_BL, 0);
    ledcWrite(0, 200);

    pinMode(TFT_RST, OUTPUT);
    digitalWrite(TFT_RST, HIGH);
    delay(10);
    digitalWrite(TFT_RST, LOW);
    delay(10);
    digitalWrite(TFT_RST, HIGH);
    delay(150);

    tft.init(240, 320);
    tft.setRotation(0);

    clear();
    drawHeader();
    drawRealTimeData();
    Serial.println("✅ Дисплей готов!");
}

void ED_Display::clear() { tft.fillScreen(_colorBg); }

void ED_Display::drawHeader()
{
    uint16_t headerBg = 0x1082;
    tft.fillRect(0, 0, 240, 28, headerBg);
    tft.drawLine(0, 27, 240, 27, 0x5208);

    tft.setTextColor(0xF9C3);
    tft.setTextSize(2);
    tft.setCursor(10, 8);
    tft.print("EnrollaDatos");

    tft.setTextColor(0xF942);
    tft.setTextSize(1);
    tft.setCursor(180, 14);
    tft.print("v1.2.0");

    tft.drawLine(0, 30, 240, 30, 0x2108);
}

void ED_Display::drawNetworkStatus()
{
    tft.fillRect(10, 40, 220, 35, 0x1082);
    tft.drawRoundRect(10, 40, 220, 35, 6, 0x5208);

    tft.setTextSize(1);
    tft.setTextColor(0x8410);
    tft.setCursor(20, 48);
    tft.print("RED");

    uint8_t clients = WiFi.softAPgetStationNum();
    bool hasClients = (clients > 0);

    tft.setTextSize(2);
    if (hasClients)
    {
        tft.setTextColor(0x07E0);
        tft.setCursor(70, 45);
        tft.print("Conectado");
        tft.fillCircle(55, 55, 4, 0x07E0);
    }
    else
    {
        tft.setTextColor(0xF800);
        tft.setCursor(70, 45);
        tft.print("Desconectado");
        tft.fillCircle(55, 55, 4, 0xF800);
    }

    tft.setTextSize(1);
    tft.setTextColor(0x8410);
    tft.setCursor(180, 48);
    tft.print("(");
    tft.print(clients);
    tft.print(")");
}

void ED_Display::_drawCard(int x, int y, int w, int h, uint16_t color)
{
    tft.drawRoundRect(x, y, w, h, 4, color);
}

void ED_Display::_drawSeparator(int y)
{
    tft.drawLine(10, y, 230, y, 0x2108);
}

void ED_Display::drawRealTimeData()
{
    tft.fillRect(0, 35, 240, 285, _colorBg);

    // Статус сети
    drawNetworkStatus();

    // Узлы (3 карточки)
    int cardY = 85;
    int cardW = 68;
    int cardH = 55;
    int gap = 8;

    // Total
    _drawCard(10, cardY, cardW, cardH, 0x8410);
    tft.setTextSize(1);
    tft.setTextColor(0x8410);
    tft.setCursor(30, cardY + 6);
    tft.print("TOTAL");
    tft.setTextSize(3);
    tft.setTextColor(0xFFFF);
    tft.setCursor(30, cardY + 18);
    tft.print(sysState.totalNodes);

    // Activos
    _drawCard(10 + cardW + gap, cardY, cardW, cardH, 0x07E0);
    tft.setTextSize(1);
    tft.setTextColor(0x07E0);
    tft.setCursor(98, cardY + 6);
    tft.print("ACTIVOS");
    tft.setTextSize(3);
    tft.setTextColor(0x07E0);
    tft.setCursor(105, cardY + 18);
    tft.print(sysState.activeNodes);

    // Dormidos
    _drawCard(10 + (cardW + gap) * 2, cardY, cardW, cardH, 0x8410);
    tft.setTextSize(1);
    tft.setTextColor(0x8410);
    tft.setCursor(170, cardY + 6);
    tft.print("DORMIDOS");
    tft.setTextSize(3);
    tft.setTextColor(0x8410);
    tft.setCursor(178, cardY + 18);
    tft.print(sysState.dormantNodes);

    _drawSeparator(cardY + cardH + 10);

    // Время передачи
    int timeY = cardY + cardH + 22;
    tft.setTextSize(1);
    tft.setTextColor(0x8410);
    tft.setCursor(20, timeY);
    tft.print("RECEPCION");
    tft.setTextSize(2);
    tft.setTextColor(0x07FF);
    tft.setCursor(20, timeY + 16);
    tft.print(sysState.lastReceive);

    tft.setTextSize(1);
    tft.setTextColor(0x8410);
    tft.setCursor(130, timeY);
    tft.print("TRANSMISION");
    tft.setTextSize(2);
    tft.setTextColor(0xFFE0);
    tft.setCursor(130, timeY + 16);
    tft.print(sysState.lastTransmit);

    _drawSeparator(timeY + 40);

    // Буфер
    int bufferY = timeY + 52;
    tft.setTextSize(1);
    tft.setTextColor(0x8410);
    tft.setCursor(20, bufferY);
    tft.print("BUFFER");

    tft.setTextSize(2);
    uint16_t bufferColor;
    if (sysState.bufferSize == 0)
        bufferColor = 0x07E0;
    else if (sysState.bufferSize < 10)
        bufferColor = 0xFFE0;
    else
        bufferColor = 0xF800;
    tft.setTextColor(bufferColor);
    tft.setCursor(20, bufferY + 16);
    tft.print(sysState.bufferSize);

    tft.setTextSize(1);
    tft.setTextColor(0x8410);
    tft.setCursor(60, bufferY + 20);
    tft.print("pkg");

    // Прогресс-бар
    int barX = 130;
    int barY = bufferY + 12;
    int barW = 90;
    int barH = 14;
    int maxBuffer = 20;
    tft.drawRect(barX, barY, barW, barH, 0x8410);
    int fill = map(sysState.bufferSize, 0, maxBuffer, 0, barW - 2);
    if (fill > 0)
    {
        tft.fillRect(barX + 1, barY + 1, fill, barH - 2, bufferColor);
    }
}

void ED_Display::setBrightness(uint8_t level)
{
    ledcWrite(0, level);
}