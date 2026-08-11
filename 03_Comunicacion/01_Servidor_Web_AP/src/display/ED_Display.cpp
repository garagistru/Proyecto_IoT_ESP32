// src/display/ED_Display.cpp
#include "ED_Display.h"
#include "ED_Config.h" // Подключаем конфигурацию (цвета, позиции, тексты)
#include "ED_State.h"  // Подключаем состояние (реальные данные)

// ============================================
// КОНСТРУКТОР
// ============================================
ED_Display::ED_Display() : tft(TFT_CS, TFT_DC, TFT_RST)
{
    // Все цвета теперь берутся из ED_Config.h
}

// ============================================
// ИНИЦИАЛИЗАЦИЯ ДИСПЛЕЯ
// ============================================
void ED_Display::begin()
{
    Serial.println("🔧 Инициализация дисплея 240x320...");

    // 1. ЯВНО УКАЗЫВАЕМ SPI ПИНЫ (SCK, MISO, MOSI, CS)
    SPI.begin(TFT_SCK, -1, TFT_MOSI, TFT_CS);

    // 2. ПОДСВЕТКА ЧЕРЕЗ PWM (ПИН 48)
    ledcSetup(0, 5000, 8);
    ledcAttachPin(TFT_BL, 0);
    ledcWrite(0, 200); // 200/255 ≈ 78% яркости

    // 3. АППАРАТНЫЙ СБРОС ДИСПЛЕЯ
    pinMode(TFT_RST, OUTPUT);
    digitalWrite(TFT_RST, HIGH);
    delay(10);
    digitalWrite(TFT_RST, LOW);
    delay(10);
    digitalWrite(TFT_RST, HIGH);
    delay(150);

    // 4. ИНИЦИАЛИЗАЦИЯ ЭКРАНА (БЕЗ SPI_MODE2, правильная ориентация 0)
    tft.init(240, 320);
    tft.setRotation(0);
    Serial.println("✅ Дисплей инициализирован (Rotation 0)");

    // 5. ОЧИСТКА И ОТРИСОВКА (без тестовых задержек)
    clear();
    drawHeader();
    Serial.println("✅ Интерфейс отрисован!");
}

// ============================================
// ОЧИСТКА ЭКРАНА
// ============================================
void ED_Display::clear()
{
    tft.fillScreen(COLOR_BG);
}

// ============================================
// ОТРИСОВКА ШАПКИ (ИСПОЛЬЗУЕТ КОНСТАНТЫ ИЗ ED_Config.h)
// ============================================
void ED_Display::drawHeader()
{
    // 1. Фон шапки
    tft.fillRect(0, 0, 240, ED_HEADER_HEIGHT, COLOR_HEADER_BG);
    tft.drawLine(0, ED_HEADER_HEIGHT - 1, 240, ED_HEADER_HEIGHT - 1, COLOR_GOLD_DIM);

    // 2. Название проекта
    tft.setTextColor(COLOR_ACCENT_LIGHT);
    tft.setTextSize(ED_TITLE_SIZE);
    tft.setCursor(ED_TITLE_X, ED_TITLE_Y);
    tft.print(ED_PROJECT_NAME);

    // 3. Версия
    tft.setTextColor(COLOR_GOLD);
    tft.setTextSize(ED_VERSION_SIZE);
    tft.setCursor(ED_VERSION_X, ED_VERSION_Y);
    tft.print(ED_PROJECT_VERSION);

    // 4. Нижняя линия
    tft.drawLine(0, 30, 240, 30, COLOR_INFO_LINE);
}

// ============================================
// ОТРИСОВКА РЕАЛЬНЫХ ДАННЫХ (ИСПОЛЬЗУЕТ ED_State.h)
// ============================================
void ED_Display::drawRealTimeData()
{
    // Очищаем только область данных (ниже шапки), чтобы не мерцал весь экран
    // tft.fillRect(0, 32, 240, 288, COLOR_BG);

    // Пример отрисовки (раскомментируй и доработай на следующем этапе):
    /*
    tft.setTextColor(COLOR_TEXT_MAIN);
    tft.setTextSize(2);
    tft.setCursor(10, 60);
    tft.print("Nodos: ");
    tft.print(sysState.activeNodes);
    tft.print("/");
    tft.print(sysState.totalNodes);
    */
}

// ============================================
// УПРАВЛЕНИЕ ЯРКОСТЬЮ
// ============================================
void ED_Display::setBrightness(uint8_t level)
{
    ledcWrite(0, level);
}