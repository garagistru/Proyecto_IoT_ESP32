// src/display/ED_Config.h
#ifndef ED_CONFIG_H
#define ED_CONFIG_H

#include <Arduino.h>

// ==========================================
// 1. СЕРВИСНАЯ ИНФОРМАЦИЯ (Меняется при прошивке)
// ==========================================
#define ED_PROJECT_NAME "EnrollaDatos"
#define ED_PROJECT_VERSION "v1.2.0"

// ==========================================
// 2. ПОЗИЦИОНИРОВАНИЕ ВЕРХНЕЙ СТРОКИ (Layout)
// Вынесено сюда, чтобы легко менять дизайн не ломая код
// ==========================================
#define ED_HEADER_HEIGHT 28 // Высота фона шапки

// Позиции для Названия проекта
#define ED_TITLE_X 10
#define ED_TITLE_Y 8
#define ED_TITLE_SIZE 2

// Позиции для Версии (выровнены по нижней границе с названием)
#define ED_VERSION_X 165 // С запасом ~3px справа от края (240)
#define ED_VERSION_Y 10  // Компенсирует разницу в рендеринге шрифтов
#define ED_VERSION_SIZE 2

// ==========================================
// 3. ЦВЕТОВАЯ ПАЛИТРА (RGB565)
// ==========================================
#define COLOR_BG 0x0000           // Черный
#define COLOR_HEADER_BG 0x1082    // Темно-фиолетовый
#define COLOR_ACCENT_LIGHT 0xF9C3 // Светлый фиолетовый (Название)
#define COLOR_GOLD 0xF942         // Золотой (Версия)
#define COLOR_GOLD_DIM 0x5208     // Темный золотой (Разделитель)
#define COLOR_INFO_LINE 0x2108    // Темно-синий (Нижняя линия)

#endif