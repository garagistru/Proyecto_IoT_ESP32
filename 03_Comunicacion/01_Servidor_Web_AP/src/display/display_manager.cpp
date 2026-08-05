#include "display_manager.h"

DisplayManager::DisplayManager()
{
    isInitialized = false;
    networkConnected = false;
    prevTotal = -1;
    prevActivos = -1;
    prevDormidos = -1;
    lastDataTimeSec = 0;
    currentTimeSec = 0;
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
    setNetworkStatus(false);

    isInitialized = true;
    Serial.println("✅ DisplayManager ARM initialized");
}

void DisplayManager::drawRightAlignedNumber(int16_t x_right, int16_t y, int value, uint8_t textSize, uint16_t color)
{
    char buf[8];
    sprintf(buf, "%d", value);
    tft->setTextColor(color, COLOR_BG);
    tft->setTextSize(textSize);

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
    // 1. Верхнее название проекта (ОТ КРАЯ ДО КРАЯ, Size 1)
    tft->setTextColor(COLOR_GRAY_LIGHT, COLOR_BG);
    tft->setTextSize(SIZE_SMALL);
    tft->setCursor(4, 10);
    tft->print(PROJECT_NAME);

    // Разделитель 1 (Y=22) - БОЛЬШОЙ ОТСТУП ПОСЛЕ ЗАГОЛОВКА
    tft->drawLine(0, 22, SCREEN_WIDTH, 22, COLOR_GRAY_DARK);

    // 2. Подписи метрик (Size 2) - НОВАЯ СЕТКА
    tft->setTextColor(COLOR_WHITE, COLOR_BG);
    tft->setTextSize(SIZE_MAIN);
    tft->setCursor(10, 80);
    tft->print(TEXT_NODOS_TOTALES);
    tft->setCursor(10, 115);
    tft->print(TEXT_ACTIVOS);
    tft->setCursor(10, 150);
    tft->print(TEXT_DORMIDOS);

    // Разделитель 2 (Y=180)
    tft->drawLine(0, 180, SCREEN_WIDTH, 180, COLOR_GRAY_DARK);

    // 3. Время - подписи (Size 2)
    tft->setTextSize(SIZE_MAIN);
    tft->setCursor(10, 200);
    tft->print(TEXT_ULTIMO_CONEXION);

    tft->setCursor(10, 230);
    tft->print(TEXT_HORA_ACTUAL);

    // 4. Нижний подвал (ОТ КРАЯ ДО КРАЯ, Size 1, Y=295)
    tft->setTextColor(COLOR_GRAY_DARK, COLOR_BG);
    tft->setTextSize(SIZE_SMALL);
    tft->setCursor(4, 295);
    tft->print(DEVICE_FULL_NAME);
}

void DisplayManager::setNetworkStatus(bool connected)
{
    if (!isInitialized)
        return;
    if (networkConnected == connected)
        return;

    networkConnected = connected;
    uint16_t textColor = connected ? COLOR_GREEN : COLOR_RED;
    const char *statusText = connected ? TEXT_STATUS_CONNECTED : TEXT_STATUS_DISCONNECTED;

    // Очищаем зону статуса (Y от 22 до 70) - БОЛЬШОЙ ОТСТУП
    tft->fillRect(0, 24, SCREEN_WIDTH, 50, COLOR_BG);

    // "Red" (Size 2, Желтый, Y=45)
    tft->setTextColor(COLOR_YELLOW, COLOR_BG);
    tft->setTextSize(SIZE_MAIN);
    tft->setCursor(10, 45);
    tft->print(DEVICE_SHORT_NAME);

    // "Conectado" / "Desconectado" (Size 2, Зеленый/Красный, Y=49)
    tft->setTextColor(textColor, COLOR_BG);
    tft->setCursor(70, 49);
    tft->print(statusText);
}

void DisplayManager::updateMetrics(int total, int activos, int dormidos)
{
    if (!isInitialized)
        return;

    // Nodos totales (Цифры Size 2, Голубые, Y=78)
    if (total != prevTotal)
    {
        tft->fillRect(140, 70, 100, 24, COLOR_BG);
        drawRightAlignedNumber(230, 78, total, SIZE_MAIN, COLOR_CYAN);
        prevTotal = total;
    }

    // Activos (Y=113)
    if (activos != prevActivos)
    {
        tft->fillRect(140, 105, 100, 24, COLOR_BG);
        drawRightAlignedNumber(230, 113, activos, SIZE_MAIN, COLOR_CYAN);
        prevActivos = activos;
    }

    // Dormidos (Y=148)
    if (dormidos != prevDormidos)
    {
        tft->fillRect(140, 140, 100, 24, COLOR_BG);
        drawRightAlignedNumber(230, 148, dormidos, SIZE_MAIN, COLOR_CYAN);
        prevDormidos = dormidos;
    }
}

void DisplayManager::updateTime(unsigned long lastDataSeconds, unsigned long currentSeconds)
{
    if (!isInitialized)
        return;

    lastDataTimeSec = lastDataSeconds;
    currentTimeSec = currentSeconds;

    // Время последней передачи (Size 2, Голубое, Y=198)
    String lastTimeStr = formatTime(lastDataSeconds);
    tft->fillRect(140, 190, 100, 24, COLOR_BG);
    tft->setTextColor(COLOR_CYAN, COLOR_BG);
    tft->setTextSize(SIZE_MAIN);

    int16_t x1, y1;
    uint16_t w, h;
    tft->getTextBounds(lastTimeStr.c_str(), 0, 0, &x1, &y1, &w, &h);
    tft->setCursor(230 - w, 198);
    tft->print(lastTimeStr);

    // Текущее время (Size 2, Голубое, Y=228)
    String currentTimeStr = formatTime(currentSeconds);
    tft->fillRect(140, 220, 100, 24, COLOR_BG);
    tft->getTextBounds(currentTimeStr.c_str(), 0, 0, &x1, &y1, &w, &h);
    tft->setCursor(230 - w, 228);
    tft->print(currentTimeStr);
}

DisplayManager display;