#ifndef DISPLAY_CONFIG_H
#define DISPLAY_CONFIG_H

// ==========================================
// ПИНЫ ПОДКЛЮЧЕНИЯ (ESP32-S3)
// ==========================================
#define TFT_CS 10
#define TFT_DC 9
#define TFT_RST 8
#define TFT_BL 48

#define SPI_SCK 12
#define SPI_MOSI 11
#define SPI_MISO -1
#define SPI_FREQ 10000000

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

// ==========================================
// ИДЕНТИФИКАЦИЯ ПРОЕКТА
// ==========================================
#define PROJECT_NAME "Arana Rastreadora de Maquinas"
#define DEVICE_SHORT_NAME "Red"
#define DEVICE_FULL_NAME "EnrollaDatos (ED) ARM v1.2.0"
#define FIRMWARE_VERSION "1.2.0"

// ==========================================
// ЦВЕТОВАЯ ПАЛИТРА (Высокий контраст)
// ==========================================
#define COLOR_BG 0x0000
#define COLOR_WHITE 0xFFFF
#define COLOR_YELLOW 0xFFE0
#define COLOR_CYAN 0x07FF
#define COLOR_GREEN 0x07E0
#define COLOR_RED 0xF800
#define COLOR_GRAY_LIGHT 0xAAAA
#define COLOR_GRAY_DARK 0x5555

// ==========================================
// РАЗМЕРЫ ШРИФТОВ
// ==========================================
#define SIZE_SMALL 1 // Для заголовка и подвала
#define SIZE_MAIN 2  // Основной (весь текст и цифры)

// ==========================================
// ТЕКСТЫ ИНТЕРФЕЙСА
// ==========================================
#define TEXT_STATUS_CONNECTED "Conectado"
#define TEXT_STATUS_DISCONNECTED "Desconectado"
#define TEXT_NODOS_TOTALES "Nodos totales:"
#define TEXT_ACTIVOS "Activos:"
#define TEXT_DORMIDOS "Dormidos:"
#define TEXT_ULTIMO_CONEXION "Ultimo conexion:"
#define TEXT_HORA_ACTUAL "Hora actual:"

#endif // DISPLAY_CONFIG_H