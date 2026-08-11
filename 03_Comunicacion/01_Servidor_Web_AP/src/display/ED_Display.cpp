// src/display/ED_Display.cpp
#include "ED_Display.h"

// ============================================
// КОНСТРУКТОР
// ============================================
ED_Display::ED_Display()
    : tft(TFT_CS, TFT_DC, TFT_RST)
{
    // Используем запрошенный оранжево-золотой оттенок как основной акцент
    _colorAccent = 0xF942;
    _colorText = 0xFFFF; // Белый
    _colorBg = 0x0000;   // Черный
}

// ============================================
// ИНИЦИАЛИЗАЦИЯ ДИСПЛЕЯ
// ============================================
void ED_Display::begin()
{
    Serial.println("🔧 Инициализация дисплея 240x320...");

    // 1. ЯВНО УКАЗЫВАЕМ SPI ПИНЫ (SCK=12, MISO=-1, MOSI=11, CS=10)
    SPI.begin(12, -1, 11, 10);

    // 2. ПОДСВЕТКА ЧЕРЕЗ PWM (ПИН 48)
    ledcSetup(0, 5000, 8);
    ledcAttachPin(TFT_BL, 0);
    ledcWrite(0, 200); // 200/255 ≈ 78% яркости (комфортно для глаз)

    // 3. АППАРАТНЫЙ СБРОС ДИСПЛЕЯ
    pinMode(TFT_RST, OUTPUT);
    digitalWrite(TFT_RST, HIGH);
    delay(10);
    digitalWrite(TFT_RST, LOW);
    delay(10);
    digitalWrite(TFT_RST, HIGH);
    delay(150);

    // 4. ИНИЦИАЛИЗАЦИЯ ЭКРАНА
    tft.init(240, 320);
    tft.setRotation(0); // Подтвержденная тобой правильная ориентация
    Serial.println("✅ Дисплей инициализирован (Rotation 0)");

    // 5. БЫСТРЫЙ ТЕСТ ЦВЕТОВ (проверка работоспособности)
    tft.fillScreen(0xF800);
    delay(300); // Красный
    tft.fillScreen(0x07E0);
    delay(300); // Зеленый
    tft.fillScreen(0x001F);
    delay(300); // Синий

    // 6. ОЧИСТКА И ОТРИСОВКА ИНТЕРФЕЙСА
    clear();
    drawHeader();
    Serial.println("✅ Интерфейс отрисован!");
}

// ============================================
// ОЧИСТКА ЭКРАНА
// ============================================
void ED_Display::clear()
{
    tft.fillScreen(_colorBg);
}

// ============================================
// ОТРИСОВКА ШАПКИ (ТВОЙ ОПТИМИЗИРОВАННЫЙ ВАРИАНТ)
// ============================================
void ED_Display::drawHeader()
{
    // Теперь ширина = 240, высота = 320. Всё рисуется корректно.

    // 1. Фон шапки (темно-фиолетовый)
    uint16_t headerBg = 0x1082;
    tft.fillRect(0, 0, 240, 28, headerBg);

    // 2. Верхняя разделительная линия внутри шапки
    uint16_t border = 0x5208;
    tft.drawLine(0, 27, 240, 27, border);

    // 3. Название проекта (светло-золотой)
    tft.setTextColor(0xF9C3);
    tft.setTextSize(2);
    tft.setCursor(10, 8);
    tft.print("EnrollaDatos");

    // 4. Версия (запрошенный цвет 0xF942)
    uint16_t versionColor = 0xF942;
    tft.setTextColor(versionColor);
    tft.setTextSize(2);

    // ВАЖНО: X=165 дает небольшой запас справа, чтобы "0" не обрезалась рамкой.
    // Y=10 идеально выравнивает нижнюю границу текста с "EnrollaDatos" (который на Y=8).
    tft.setCursor(164, 9);
    tft.print("v1.2.0");

    // 5. Тонкая декоративная линия под всей шапкой
    tft.drawLine(0, 30, 240, 30, 0x2108);
}

// ============================================
// УПРАВЛЕНИЕ ЯРКОСТЬЮ
// ============================================
void ED_Display::setBrightness(uint8_t level)
{
    ledcWrite(0, level);
}