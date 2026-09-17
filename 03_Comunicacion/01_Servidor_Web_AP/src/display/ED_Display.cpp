// src/display/ED_Display.cpp
#include "ED_Display.h"
#include "ED_State.h"
#include <WiFi.h>
#include "../ED_Utils.h"

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
    LOGLN("Display init...");

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
    LOGLN("Display ready");
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
    tft.setCursor(170, 14); // ← сдвинули чуть левее (v1.3.2 = 6 символов, не 5)
    tft.print(ED_FULL_VERSION);

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

    // ========================================
    // 1. СТАТУС СЕТИ
    // ========================================
    drawNetworkStatus();

    // ========================================
    // 2. NODOS
    // ========================================
    int cardY = 85;
    int cardW = 68;
    int cardH = 55;
    int gap = 8;

    // TOTAL
    _drawCard(10, cardY, cardW, cardH, 0x8410);
    tft.setTextSize(1);
    tft.setTextColor(0x8410);
    tft.setCursor(30, cardY + 6);
    tft.print("TOTAL");
    tft.setTextSize(3);
    tft.setTextColor(0xFFFF);
    tft.setCursor(30, cardY + 18);
    tft.print(sysState.totalNodes);

    // ACTIVOS
    _drawCard(10 + cardW + gap, cardY, cardW, cardH, 0x07E0);
    tft.setTextSize(1);
    tft.setTextColor(0x07E0);
    tft.setCursor(98, cardY + 6);
    tft.print("ACTIVOS");
    tft.setTextSize(3);
    tft.setTextColor(0x07E0);
    tft.setCursor(105, cardY + 18);
    tft.print(sysState.activeNodes);

    // DORMIDOS
    _drawCard(10 + (cardW + gap) * 2, cardY, cardW, cardH, 0x8410);
    tft.setTextSize(1);
    tft.setTextColor(0x8410);
    tft.setCursor(170, cardY + 6);
    tft.print("DORMIDOS");
    tft.setTextSize(3);
    tft.setTextColor(0x8410);
    tft.setCursor(178, cardY + 18);
    tft.print(sysState.dormantNodes);

    // ========================================
    // 3. РАЗДЕЛИТЕЛЬ 1
    // ========================================
    _drawSeparator(155);

    // ========================================
    // 4. RECIBE / ENVIA — ДВЕ СТРОКИ
    // ========================================

    // --- Строка 1: labels ---
    tft.setTextSize(2);

    // RECIBE — cyan
    tft.setTextColor(0x07FF);
    tft.setCursor(52, 168);
    tft.print("RECIBE");

    // ENVIA — жёлтый
    tft.setTextColor(0xFFE0);
    tft.setCursor(140, 168);
    tft.print("ENVIA");

    // --- Строка 2: "hace:" + значения ---
    // "hace:" — зелёный, size 2
    tft.setTextSize(2);
    tft.setTextColor(0x07E0);
    tft.setCursor(8, 198);
    tft.print("hace:");

    // Значение RECIBE — cyan, центрируем под RECIBE (52-124, центр 88)
    // "17m" ширина ~36px, x = 88 - 18 = 70
    tft.setTextColor(0x07FF);
    tft.setCursor(70, 198);
    tft.print(sysState.lastReceive);

    // Значение ENVIA — жёлтый, центрируем под ENVIA (140-200, центр 170)
    // "3s" ширина ~24px, x = 170 - 12 = 158
    tft.setTextColor(0xFFE0);
    tft.setCursor(158, 198);
    tft.print(sysState.lastTransmit);

    // ========================================
    // 5. РАЗДЕЛИТЕЛЬ 2
    // ========================================
    _drawSeparator(225);

    // ========================================
    // 6. HUCHA (Буфер передачи) - ПО ЦЕНТРУ
    // ========================================

    // --- Заголовок "HUCHA" по центру (size 2) ---
    tft.setTextSize(2);
    tft.setTextColor(0x8410); // Темно-зеленый/оливковый для заголовка
    // 6 символов * 12px = 72px. (240 - 72) / 2 = 84
    tft.setCursor(84, 240);
    tft.print("HUCHA");

    // --- Значение + paq + бар ниже ---
    uint16_t bufferColor;
    if (sysState.bufferSize == 0)
        bufferColor = 0x07E0; // Зеленый (пусто/ок)
    else if (sysState.bufferSize < 10)
        bufferColor = 0xFFE0; // Желтый (внимание)
    else
        bufferColor = 0xF800; // Красный (переполнение)

    // Значение — size 2
    tft.setTextSize(2);
    tft.setTextColor(bufferColor);
    tft.setCursor(20, 268);
    tft.print(sysState.bufferSize);

    // "paq" (paquetes) — size 2, цвет заголовка
    tft.setTextColor(0x8410);
    tft.setCursor(50, 268);
    tft.print("paq");

    // Прогресс-бар
    int barX = 120;
    int barY = 268;
    int barW = 110;
    int barH = 16;
    int maxBuffer = 20; // Настрой под свой реальный максимум

    // Рамка бара
    tft.drawRect(barX, barY, barW, barH, 0x8410);

    // Заполнение бара
    int fill = map(sysState.bufferSize, 0, maxBuffer, 0, barW - 2);
    if (fill > 0)
    {
        tft.fillRect(barX + 1, barY + 1, fill, barH - 2, bufferColor);
    }
    // ========================================
    // 7. ПОДПИСЬ (footer)
    // ========================================
    tft.drawLine(10, 295, 230, 295, 0x4208);

    tft.setTextSize(1);
    // tft.setTextColor(0x8410);
    tft.setTextColor(0xF942);
    tft.setCursor(66, 305);
    tft.print("hecho por Mecanico");
}

void ED_Display::setBrightness(uint8_t level)
{
    ledcWrite(0, level);
}