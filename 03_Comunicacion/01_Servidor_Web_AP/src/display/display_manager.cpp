#include "display_manager.h"

DisplayManager::DisplayManager()
{
    isInitialized = false;
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

    // Аппаратный сброс
    pinMode(TFT_RST, OUTPUT);
    digitalWrite(TFT_RST, HIGH);
    delay(10);
    digitalWrite(TFT_RST, LOW);
    delay(10);
    digitalWrite(TFT_RST, HIGH);
    delay(150);

    tft->init(TFT_WIDTH, TFT_HEIGHT);
    if (TFT_INVERSION)
    {
        tft->invertDisplay(true);
    }

    drawStaticBackground();
    isInitialized = true;
    Serial.println("✅ DisplayManager инициализирован");
}

void DisplayManager::drawStaticBackground()
{
    tft->fillScreen(ST77XX_BLACK);
    tft->setTextColor(ST77XX_GREEN);
    tft->setTextSize(2);
    tft->setCursor(30, 20);
    tft->print("SISTEMA IoT");

    tft->drawRect(10, 60, 220, 180, ST77XX_BLUE);

    tft->setTextColor(ST77XX_WHITE);
    tft->setTextSize(1);
    tft->setCursor(25, 75);
    tft->print("DATOS DEL SENSOR:");
    tft->setCursor(25, 140);
    tft->print("TEMPERATURA:");
    tft->setCursor(25, 200);
    tft->print("HUMEDAD:");
}

void DisplayManager::setStatus(bool isActive)
{
    tft->fillRect(150, 15, 80, 30, ST77XX_BLACK);
    tft->setTextSize(1);
    if (isActive)
    {
        tft->fillCircle(160, 30, 8, ST77XX_GREEN);
        tft->setTextColor(ST77XX_GREEN);
        tft->setCursor(175, 25);
        tft->print("ACTIVO");
    }
    else
    {
        tft->fillCircle(160, 30, 8, ST77XX_RED);
        tft->setTextColor(ST77XX_RED);
        tft->setCursor(175, 25);
        tft->print("SIN DATOS");
    }
}

void DisplayManager::clearDataZone()
{
    tft->fillRect(12, 90, 216, 148, ST77XX_BLACK);
}

void DisplayManager::updateSensorData(String sensorName, float temperature, float humidity, int requestCount)
{
    clearDataZone();

    if (sensorName.length() > 18)
        sensorName = sensorName.substring(0, 18);

    tft->setTextColor(ST77XX_YELLOW);
    tft->setTextSize(2);
    tft->setCursor(25, 95);
    tft->print(sensorName);

    tft->setTextColor(ST77XX_RED);
    tft->setTextSize(4);
    tft->setCursor(25, 145);
    tft->print(temperature, 1);
    tft->setTextSize(2);
    tft->drawCircle(135, 160, 4, ST77XX_RED);

    tft->setTextColor(ST77XX_CYAN);
    tft->setTextSize(4);
    tft->setCursor(25, 205);
    tft->print(humidity, 1);
    tft->setTextSize(2);
    tft->setCursor(145, 215);
    tft->print("%");

    tft->setTextColor(ST77XX_WHITE);
    tft->setTextSize(1);
    tft->setCursor(25, 260);
    tft->print("Peticiones: ");
    tft->print(requestCount);
}

void DisplayManager::showError(String message)
{
    tft->fillScreen(ST77XX_BLACK);
    tft->setTextColor(ST77XX_RED);
    tft->setTextSize(2);
    tft->setCursor(20, 100);
    tft->print("ERROR:");
    tft->setTextSize(1);
    tft->setCursor(20, 130);
    tft->print(message);
}

// Создаем глобальный объект
DisplayManager display;
// ================== НОВЫЕ ФУНКЦИИ ДЛЯ МЕНЮ ==================

void DisplayManager::setScreen(DisplayScreen screen)
{
    currentScreen = screen;
    tft->fillScreen(COLOR_BG);

    if (screen == SCREEN_DASHBOARD)
    {
        drawDashboard();
    }
    else if (screen == SCREEN_NODE_LIST)
    {
        drawNodeList();
    }
}

void DisplayManager::drawDashboard()
{
    // Заголовок с версией
    tft->setTextColor(COLOR_GREEN, COLOR_BG);
    tft->setTextSize(2);
    tft->setCursor(12, 25);
    tft->print(DEVICE_NAME);
    tft->setTextSize(1);
    tft->print(" v");
    tft->print(FIRMWARE_VERSION);

    drawStaticBackground(); // Рисуем рамку и подписи
}

void DisplayManager::updateNodeCount(int activeCount, int sleepCount)
{
    if (currentScreen != SCREEN_DASHBOARD)
        return;

    // Проверяем, изменились ли значения
    if (activeCount == prevActiveCount && sleepCount == prevSleepCount)
    {
        return;
    }

    // Очищаем зоны цифр
    tft->fillRect(12, 96, 216, 100, COLOR_ORANGE);
    tft->fillRect(12, 206, 216, 100, COLOR_BLUE);

    // Рисуем карточку ACTIVOS
    tft->setTextColor(COLOR_WHITE, COLOR_ORANGE);
    tft->setTextSize(4); // Крупный шрифт для цифры
    int16_t x1 = getCenteredX(String(activeCount), 216, 4);
    tft->setCursor(x1, 145);
    tft->print(activeCount);

    tft->setTextSize(2);
    int16_t x2 = getCenteredX("SENSORES ACTIVOS", 216, 2);
    tft->setCursor(x2, 185);
    tft->print("SENSORES ACTIVOS");

    // Рисуем карточку EN ESPERA
    tft->setTextColor(COLOR_WHITE, COLOR_BLUE);
    tft->setTextSize(4);
    int16_t x3 = getCenteredX(String(sleepCount), 216, 4);
    tft->setCursor(x3, 255);
    tft->print(sleepCount);

    tft->setTextSize(2);
    int16_t x4 = getCenteredX("EN ESPERA", 216, 2);
    tft->setCursor(x4, 295);
    tft->print("EN ESPERA");

    prevActiveCount = activeCount;
    prevSleepCount = sleepCount;
    lastUpdateTime = millis();
}

void DisplayManager::updateLastSeen(unsigned long secondsAgo)
{
    // Обновляем подвал с временем
    tft->fillRect(12, 305, 216, 15, COLOR_BG);
    tft->setTextColor(COLOR_GRAY, COLOR_BG);
    tft->setTextSize(1);
    tft->setCursor(12, 315);
    tft->print("Update: ");
    tft->print(secondsAgo);
    tft->print("s ago");
}

void DisplayManager::drawNodeList()
{
    // Заголовок
    tft->setTextColor(COLOR_WHITE, COLOR_BG);
    tft->setTextSize(2);
    tft->setCursor(12, 25);
    tft->print("NODOS CONECTADOS");

    // Разделитель
    tft->drawLine(12, 45, 228, 45, COLOR_GRAY);

    // Здесь будет список узлов (заглушка)
    tft->setTextColor(COLOR_GRAY, COLOR_BG);
    tft->setTextSize(1);
    tft->setCursor(12, 100);
    tft->print("Cargando nodos...");
}

int16_t DisplayManager::getCenteredX(String text, int16_t maxWidth, uint8_t textSize)
{
    // Простая эвристика для центрирования
    int16_t textWidth = text.length() * 6 * textSize; // Примерная ширина
    return (240 - textWidth) / 2;
}