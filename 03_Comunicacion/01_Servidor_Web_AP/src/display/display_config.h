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
// ИДЕНТИФИКАЦИЯ ПРОЕКТА (Масштабируемо!)
// ==========================================
#define PROJECT_NAME "Araña Rastreadora de Máquinas"
#define DEVICE_SHORT_NAME "ED"
#define DEVICE_FULL_NAME "EnrollaDatos (ED) A.R.M.-v1.2.0"
#define FIRMWARE_VERSION "1.2.0"

// ==========================================
// ЦВЕТОВАЯ ПАЛИТРА (RGB565)
// ==========================================
#define COLOR_BG 0x0A2A         // Глубокий темный фон (#0A2A4A)
#define COLOR_LIGHT_BLUE 0x6DBF // Светло-голубой для заголовка
#define COLOR_YELLOW 0xFFE0     // Желтый для текста и цифр
#define COLOR_GREEN 0x07E0      // Зеленый (Conectado, Activos)
#define COLOR_RED 0xF800        // Красный (Desconectado)
#define COLOR_GRAY 0x8888       // Серый (Dormidos, разделители)
#define COLOR_TIME 0xCCCC       // Светло-серый для времени
#define COLOR_FOOTER 0xAAAA     // Темно-серый для подвала

// ==========================================
// ТЕКСТЫ ИНТЕРФЕЙСА
// ==========================================
#define TEXT_STATUS_CONNECTED "Conectado"
#define TEXT_STATUS_DISCONNECTED "Desconectado"
#define TEXT_NODOS_TOTALES "Nodos totales:"
#define TEXT_ACTIVOS "Activos:"
#define TEXT_DORMIDOS "Dormidos:"
#define TEXT_ULTIMA_ACT "Última actualización:"

#endif // DISPLAY_CONFIG_H