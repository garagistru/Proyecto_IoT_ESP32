#include "display_manager.h"

// Спрайт паука 16x16 (1 бит на пиксель). Рисуется цветом, который мы передадим.
const uint8_t spider_bmp[] PROGMEM = {
    0x00, 0x00, //
    0x08, 0x80, //   🕷️
    0x0C, 0xC0, //  🕷️🕷️
    0x1E, 0xE0, // 🕷️🕷️🕷️
    0x1F, 0xF0, // 🕷️🕷️🕷️🕷️
    0x0F, 0xF0, //  🕷️🕷️🕷️🕷️
    0x07, 0xE0, //   🕷️🕷️🕷️
    0x00, 0x00, //
    0x0C, 0xC0, //  🕷️  🕷️
    0x18, 0x30, // 🕷️    🕷️
    0x10, 0x10, // 🕷️    🕷️
    0x00, 0x00, //
    0x00, 0x00, //
    0x00, 0x00, //
    0x00, 0x00, //
    0x00, 0x00  //
};

DisplayManager::DisplayManager()
{
    isInitialized = false;
    networkConnected = false;
    prevTotal = -1;
    prevActivos = -1;
    prevDormidos = -1;
    lastUpdateTimeSec = 999999; // Force first draw
}

void DisplayManager::init()
{
    if (isInitialized)
        return;

    tft = new Adafruit_ST7789(&SPI, TFT_CS, TFT_DC, TFT_RST);

    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, TFT_CS);
    SPI.setFrequency(SPI_FREQ);

    pinMode(TFT_RST, OUTPUT);
    digitalWrite(TFT_RST, HIGH);
    delay(10);
    digitalWrite(TFT_RST, LOW);
    delay(10);
    digitalWrite(TFT_RST, HIGH);
    delay(150);

    tft->init(SCREEN_WIDTH, SCREEN_HEIGHT);
    tft->invertDisplay(true);

    tft->fillScreen(COLOR_BG);
    drawStaticUI();
    setNetworkStatus(false); // По умолчанию

    isInitialized = true;
    Serial.println("✅ DisplayManager ARM initialized");
}

void DisplayManager::drawSpider(uint16_t x, uint16_t y, uint16_t color)
{
    // Рисуем монохромный битмап, закрашивая его нужным цветом (зеленым или красным)
    tft->drawBitmap(x, y, spider_bmp, 16, 16, color);
}

void DisplayManager::drawRightAlignedNumber(int16_t x_right, int16_t y, int value, uint16_t color)
{
    char buf[8];
    sprintf(buf, "%d", value);
    tft->setTextColor(color, COLOR_BG);
    tft->setTextSize(2); // ~12pt эквивалент

    // Вычисляем ширину текста для правого выравнивания
    int16_t x1, y1;
    uint16_t w, h;
    tft->getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);

    tft->setCursor(x_right - w, y);
    tft->print(buf);
}

String DisplayManager::formatTime(unsigned long totalSeconds)
{
    unsigned long h = (totalSeconds / 3600) % 24;
    unsigned long m = (totalSeconds / 60) % 60;
    unsigned long s = totalSeconds % 60;
    char timeStr[9];
    sprintf(timeStr, "%02lu:%02lu:%02lu", h, m, s);
    return String(timeStr);
}

void DisplayManager::drawStaticUI()
{
    // 1. Верхнее название проекта (по центру)
    tft->setTextColor(COLOR_LIGHT_BLUE, COLOR_BG);
    tft->setTextSize(1); // ~9pt
    int16_t x1, y1;
    uint16_t w, h;
    tft->getTextBounds(PROJECT_NAME, 0, 0, &x1, &y1, &w, &h);
    tft->setCursor((SCREEN_WIDTH - w) / 2, 14);
    tft->print(PROJECT_NAME);

    // 2. Краткое имя "ED"
    tft->setTextColor(COLOR_YELLOW, COLOR_BG);
    tft->setTextSize(2); // ~14pt
    tft->setCursor(30, 36);
    tft->print(DEVICE_SHORT_NAME);

    // 3. Статика "Estado:"
    tft->setTextSize(1);
    tft->setCursor(120, 38);
    tft->print("Estado:");

    // 4. Разделитель 1
    tft->drawLine(16, 58, 224, 58, COLOR_GRAY);

    // 5. Подписи метрик
    tft->setTextColor(COLOR_YELLOW, COLOR_BG);
    tft->setTextSize(1);
    tft->setCursor(16, 80);
    tft->print(TEXT_NODOS_TOTALES);
    tft->setCursor(16, 115);
    tft->print(TEXT_ACTIVOS);
    tft->setCursor(16, 150);
    tft->print(TEXT_DORMIDOS);

    // 6. Разделитель 2
    tft->drawLine(16, 185, 224, 185, COLOR_GRAY);

    // 7. Подпись времени
    tft->setTextColor(COLOR_TIME, COLOR_BG);
    tft->setCursor(16, 210);
    tft->print(TEXT_ULTIMA_ACT);

    // 8. Нижний подвал
    tft->setTextColor(COLOR_FOOTER, COLOR_BG);
    tft->setCursor(16, 305);
    tft->print(DEVICE_FULL_NAME);
}

void DisplayManager::setNetworkStatus(bool connected)
{
    if (!isInitialized)
        return;
    if (networkConnected == connected)
        return; // Не перерисовываем, если не изменилось

    networkConnected = connected;
    uint16_t spiderColor = connected ? COLOR_GREEN : COLOR_RED;
    uint16_t textColor = connected ? COLOR_GREEN : COLOR_RED;
    const char *statusText = connected ? TEXT_STATUS_CONNECTED : TEXT_STATUS_DISCONNECTED;

    // Рисуем паука
    drawSpider(4, 28, spiderColor);

    // Стираем и рисуем текст статуса
    tft->fillRect(120, 30, 120, 20, COLOR_BG);
    tft->setTextColor(textColor, COLOR_BG);
    tft->setTextSize(1);
    tft->setCursor(120, 42);
    tft->print(statusText);
}

void DisplayManager::updateMetrics(int total, int activos, int dormidos)
{
    if (!isInitialized)
        return;

    // Nodos totales
    if (total != prevTotal)
    {
        tft->fillRect(160, 70, 80, 20, COLOR_BG); // Очистка
        drawRightAlignedNumber(224, 78, total, COLOR_YELLOW);
        prevTotal = total;
    }

    // Activos
    if (activos != prevActivos)
    {
        tft->fillRect(160, 105, 80, 20, COLOR_BG);
        drawRightAlignedNumber(224, 113, activos, COLOR_YELLOW);
        // Зеленый кружок
        tft->fillCircle(234, 118, 5, COLOR_GREEN);
        prevActivos = activos;
    }

    // Dormidos
    if (dormidos != prevDormidos)
    {
        tft->fillRect(160, 140, 80, 20, COLOR_BG);
        drawRightAlignedNumber(224, 148, dormidos, COLOR_YELLOW);
        // Серый кружок (обводка для стиля "○")
        tft->drawCircle(234, 153, 5, COLOR_GRAY);
        prevDormidos = dormidos;
    }
}

void DisplayManager::updateTime(unsigned long secondsSinceMidnight)
{
    if (!isInitialized)
        return;
    if (secondsSinceMidnight == lastUpdateTimeSec)
        return;

    String timeStr = formatTime(secondsSinceMidnight);

    // Очищаем только область времени
    tft->fillRect(160, 200, 80, 20, COLOR_BG);
    tft->setTextColor(COLOR_TIME, COLOR_BG);
    tft->setTextSize(1);

    int16_t x1, y1;
    uint16_t w, h;
    tft->getTextBounds(timeStr.c_str(), 0, 0, &x1, &y1, &w, &h);
    tft->setCursor(224 - w, 210); // Правое выравнивание
    tft->print(timeStr);

    lastUpdateTimeSec = secondsSinceMidnight;
}

DisplayManager display;