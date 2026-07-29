#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// ==========================================
// 1. НАСТРОЙКИ ПИНОВ (ПРОВЕРЕННЫЕ И БЕЗОПАСНЫЕ)
// ==========================================
#define TFT_CS 10
#define TFT_DC 9
#define TFT_RST 8
#define TFT_BL 48

// Создаем объект дисплея с явным указанием SPI
Adafruit_ST7789 tft = Adafruit_ST7789(&SPI, TFT_CS, TFT_DC, TFT_RST);

// Настройки сети
const char *ssid_ap = "Mechanic";
const char *password_ap = "12345678";
WebServer server(80);

// Переменные для хранения последних данных
String lastSensorName = "Ожидание...";
float lastTemp = 0.0;
float lastHum = 0.0;
int requestCount = 0;
unsigned long lastDataTime = 0;
bool isSystemActive = false;

// ==========================================
// 2. ФУНКЦИИ ДЛЯ ДИСПЛЕЯ
// ==========================================

void drawStaticBackground()
{
  // Рисуем фон только один раз при старте
  tft.fillScreen(ST77XX_BLACK);

  // Заголовок
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.setCursor(30, 20);
  tft.print("SISTEMA IoT");

  // Рамка для данных
  tft.drawRect(10, 60, 220, 180, ST77XX_BLUE);

  // Статичные подписи
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(25, 75);
  tft.print("DATOS DEL SENSOR:");
  tft.setCursor(25, 140);
  tft.print("TEMPERATURA:");
  tft.setCursor(25, 200);
  tft.print("HUMEDAD:");
}

void updateStatus(bool isActive)
{
  // Стираем старую зону статуса
  tft.fillRect(150, 15, 80, 30, ST77XX_BLACK);

  tft.setTextSize(1);
  if (isActive)
  {
    tft.fillCircle(160, 30, 8, ST77XX_GREEN);
    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(175, 25);
    tft.print("ACTIVO");
  }
  else
  {
    tft.fillCircle(160, 30, 8, ST77XX_RED);
    tft.setTextColor(ST77XX_RED);
    tft.setCursor(175, 25);
    tft.print("SIN DATOS");
  }
}

void clearDataZone()
{
  // Черный прямоугольник внутри рамки для стирания старых цифр
  tft.fillRect(12, 90, 216, 148, ST77XX_BLACK);
}

void drawData(String name, float temp, float hum)
{
  clearDataZone(); // Сначала "выключаем" старое

  // Имя датчика
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(2);
  tft.setCursor(25, 95);
  tft.print(name);

  // Температура (Крупно, Красный)
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(4);
  tft.setCursor(25, 145);
  tft.print(temp, 1);
  tft.setTextSize(2);
  tft.drawCircle(135, 160, 4, ST77XX_RED); // Значок градуса

  // Влажность (Крупно, Голубой)
  tft.setTextColor(ST77XX_CYAN); // В Adafruit это часто ST77XX_BLUE или ST77XX_CYAN
  tft.setTextSize(4);
  tft.setCursor(25, 205);
  tft.print(hum, 1);
  tft.setTextSize(2);
  tft.setCursor(145, 215);
  tft.print("%");

  // Счетчик запросов
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(25, 260);
  tft.print("Peticiones: ");
  tft.print(requestCount);
}

// ==========================================
// 3. SETUP
// ==========================================
void setup()
{
  Serial.begin(115200);
  delay(3000); // Ждем стабилизации
  Serial.println("=== ЗАПУСК ПОЛНОЙ СИСТЕМЫ ===");

  // 1. Инициализация дисплея (твоя проверенная последовательность)
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH); // Сразу включаем подсветку

  SPI.begin(12, -1, 11, TFT_CS); // SCK=12, MISO=-1, MOSI=11, CS=10
  SPI.setFrequency(10000000);    // 10 МГц для надежности

  pinMode(TFT_RST, OUTPUT);
  digitalWrite(TFT_RST, HIGH);
  delay(10);
  digitalWrite(TFT_RST, LOW);
  delay(10);
  digitalWrite(TFT_RST, HIGH);
  delay(150);

  tft.init(240, 320);
  tft.invertDisplay(true); // Критично для твоего модуля

  drawStaticBackground();
  updateStatus(false); // Показываем "SIN DATOS"
  Serial.println("✅ Дисплей инициализирован");

  // 2. Инициализация Wi-Fi
  WiFi.softAP(ssid_ap, password_ap);
  Serial.print("✅ Точка доступа создана: ");
  Serial.println(WiFi.softAPIP());

  // 3. Настройка веб-сервера
  server.on("/data", HTTP_POST, []()
            {
    if (server.hasArg("nombre") && server.hasArg("temperatura") && server.hasArg("humedad")) {
      lastSensorName = server.arg("nombre");
      lastTemp = server.arg("temperatura").toFloat();
      lastHum = server.arg("humedad").toFloat();
      requestCount++;
      lastDataTime = millis();
      
      if (!isSystemActive) {
        isSystemActive = true;
        updateStatus(true); // ВКЛЮЧАЕМ зеленый статус
      }
      
      drawData(lastSensorName, lastTemp, lastHum); // Рисуем новые данные на экран
      
      Serial.printf("📊 Данные получены: %s | T: %.1f | H: %.1f\n", lastSensorName.c_str(), lastTemp, lastHum);
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "ERROR");
    } });

  server.begin();
  Serial.println("🚀 HTTP сервер запущен и ждет данные");
}

// ==========================================
// 4. LOOP
// ==========================================
void loop()
{
  server.handleClient();

  // Таймаут: если данных нет 30 секунд, возвращаем в режим ожидания
  if (isSystemActive && (millis() - lastDataTime > 30000))
  {
    isSystemActive = false;
    updateStatus(false);
    clearDataZone();
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);
    tft.setCursor(40, 150);
    tft.print("ESPERANDO...");
  }

  delay(10);
}