#ifndef DISPLAY_CONFIG_H
#define DISPLAY_CONFIG_H

// Pines de conexión
#define TFT_CS   10
#define TFT_DC   9
#define TFT_RST  8
#define TFT_BL   48

// Parámetros SPI
#define SPI_SCK  12
#define SPI_MOSI 11
#define SPI_MISO -1
#define SPI_FREQ 10000000

// Dimensiones
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320

// Versión y Marca
#define FIRMWARE_VERSION "1.2.0"
#define DEVICE_BRAND "ARM"

// Paleta de colores (RGB565)
#define COLOR_BG        0x1A1A
#define COLOR_GREEN     0x07E0
#define COLOR_RED       0xF800
#define COLOR_ORANGE    0xFD20
#define COLOR_BLUE      0x345A
#define COLOR_WHITE     0xFFFF
#define COLOR_GRAY      0x8888
#define COLOR_YELLOW    0xFFE0

// Textos de interfaz
#define TEXT_STATUS_CONNECTED "RED CONECTADA"
#define TEXT_STATUS_DISCONNECTED "RED CAIDA"
#define TEXT_ACTIVOS "ACTIVOS"
#define TEXT_DORMIDOS "DORMIDOS"
#define TEXT_UPDATE_PREFIX "Update: "
#define TEXT_UPDATE_SUFFIX "s"

#endif // DISPLAY_CONFIG_H