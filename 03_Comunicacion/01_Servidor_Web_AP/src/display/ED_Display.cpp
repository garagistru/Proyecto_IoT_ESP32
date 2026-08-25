// src/display/ED_Display.cpp
#include "ED_Display.h"
#include "display/ED_State.h"
#include <WiFi.h>

// ============================================
// КОНСТРУКТОР
// ============================================
ED_Display::ED_Display()
    : tft(TFT_CS, TFT_DC, TFT_RST)
{
    _colorAccent = COLOR_ACCENT;
    _colorText = COLOR_WHITE;
    _colorBg = COLOR_BG;
}

// ============================================
// ИНИЦИАЛИЗАЦИЯ
// ============================================
void ED_Display::begin()
{
    Serial.println("🔧 Инициализация дисплея...");

    // --- SPI ---
    SPI.begin(TFT_SCK, -1, TFT_MOSI, TFT_CS);

    // --- Подсветка ---
    ledcSetup(0, 5000, 8);
    ledcAttachPin(TFT_BL, 0);
    ledcWrite(0, 200);

    // --- Сброс ---
    pinMode(TFT_RST, OUTPUT);
    digitalWrite(TFT_RST, HIGH);
    delay(10);
    digitalWrite(TFT_RST, LOW);
    delay(10);
    digitalWrite(TFT_RST, HIGH);
    delay(150);

    // --- Экран ---
    tft.init(240, 320);
    tft.setRotation(0);

    clear();
    drawHeader();
    drawRealTimeData();
    Serial.println("✅ Дисплей готов!");
}

// ============================================
// ОЧИСТКА
// ============================================
void ED_Display::clear()
{
    tft.fillScreen(_colorBg);
}

// ============================================
// ХЕДЕР
// ============================================
void ED_Display::drawHeader()
{
    // --- Фон ---
    tft.fillRect(0, 0, 240, 32, 0x1082);
    tft.drawLine(0, 31, 240, 31, 0x5208);

    // --- Название ---
    tft.setTextColor(0xF9C3);
    tft.setTextSize(2);
    tft.setCursor(12, 10);
    tft.print("EnrollaDatos");

    // --- Версия ---
    tft.setTextColor(0xF942);
    tft.setTextSize(1);
    tft.setCursor(180, 14);
    tft.print("v1.2.0");

    // --- Тонкая линия ---
    tft.drawLine(0, 34, 240, 34, 0x2108);
}

// ============================================
// ОТРИСОВКА СТАТУСА СЕТИ
// ============================================
void ED_Display::drawNetworkStatus()
{
    // --- Зона статуса (Y=40-70) ---
    tft.fillRect(10, 40, 220, 35, 0x1082);
    tft.drawRoundRect(10, 40, 220, 35, 6, 0x5208);

    // --- Метка ---
    tft.setTextSize(1);
    tft.setTextColor(0x8410);
    tft.setCursor(20, 48);
    tft.print("RED");

    // --- Статус ---
    uint8_t clients = WiFi.softAPgetStationNum();
    bool hasClients = (clients > 0);

    tft.setTextSize(2);
    if (hasClients)
    {
        tft.setTextColor(0x07E0); // Зеленый
        tft.setCursor(70, 45);
        tft.print("Conectado");

        // Индикатор (зеленый круг)
        tft.fillCircle(55, 55, 4, 0x07E0);
    }
    else
    {
        tft.setTextColor(0xF800); // Красный
        tft.setCursor(70, 45);
        tft.print("Desconectado");

        // Индикатор (красный круг)
        tft.fillCircle(55, 55, 4, 0xF800);
    }

    // --- Количество клиентов ---
    tft.setTextSize(1);
    tft.setTextColor(0x8410);
    tft.setCursor(180, 48);
    tft.print("(");
    tft.print(clients);
    tft.print(")");
}

// ============================================
// ОТРИСОВКА КАРТОЧКИ УЗЛА
// ============================================
void ED_Display::_drawCard(int x, int y, int w, int h, uint16_t color)
{
    tft.drawRoundRect(x, y, w, h, 4, color);
}

// ============================================
// ОТРИСОВКА СЕПАРАТОРА
// ============================================
void ED_Display::_drawSeparator(int y)
{
    tft.drawLine(10, y, 230, y, 0x2108);
}

// ============================================
// ОСНОВНОЙ МЕТОД ОТРИСОВКИ
// ============================================
void ED_Display::drawRealTimeData()
{
    // --- Очищаем основную область ---
    tft.fillRect(0, 35, 240, 285, _colorBg);

    // --- 1. СТАТУС СЕТИ ---
    drawNetworkStatus();

    // --- 2. УЗЛЫ (3 КАРТОЧКИ) ---
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

    // --- 3. СЕПАРАТОР ---
    _drawSeparator(cardY + cardH + 10);

    // --- 4. ВРЕМЯ ПЕРЕДАЧИ ---
    int timeY = cardY + cardH + 22;

    // Recepción
    tft.setTextSize(1);
    tft.setTextColor(0x8410);
    tft.setCursor(20, timeY);
    tft.print("RECEPCION");
    tft.setTextSize(2);
    tft.setTextColor(0x07FF); // Голубой
    tft.setCursor(20, timeY + 16);
    tft.print(sysState.lastReceive);

    // Transmisión
    tft.setTextSize(1);
    tft.setTextColor(0x8410);
    tft.setCursor(130, timeY);
    tft.print("TRANSMISION");
    tft.setTextSize(2);
    tft.setTextColor(0xFFE0); // Желтый
    tft.setCursor(130, timeY + 16);
    tft.print(sysState.lastTransmit);

    // --- 5. СЕПАРАТОР ---
    _drawSeparator(timeY + 40);

    // --- 6. БУФЕР ---
    int bufferY = timeY + 52;

    tft.setTextSize(1);
    tft.setTextColor(0x8410);
    tft.setCursor(20, bufferY);
    tft.print("BUFFER");

    tft.setTextSize(2);
    uint16_t bufferColor;
    if (sysState.bufferSize == 0)
    {
        bufferColor = 0x07E0; // Зеленый
    }
    else if (sysState.bufferSize < 10)
    {
        bufferColor = 0xFFE0; // Желтый
    }
    else
    {
        bufferColor = 0xF800; // Красный
    }
    tft.setTextColor(bufferColor);
    tft.setCursor(20, bufferY + 16);
    tft.print(sysState.bufferSize);

    tft.setTextSize(1);
    tft.setTextColor(0x8410);
    tft.setCursor(60, bufferY + 20);
    tft.print("pkg");

    // --- Прогресс-бар буфера (визуальный) ---
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

// ============================================
// ЯРКОСТЬ
// ============================================
void ED_Display::setBrightness(uint8_t level)
{
    ledcWrite(0, level);
}