#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "display/display_manager.h"

const char *ssid_ap = "Mechanic";
const char *password_ap = "12345678";
WebServer server(80);

// Счетчики
int activeNodeCount = 0;
int sleepNodeCount = 0;
unsigned long lastActivityTime = 0;

// Простой таймаут для демонстрации
#define NODE_TIMEOUT 30000 // 30 секунд

void setup()
{
  Serial.begin(115200);
  delay(3000);
  Serial.println("=== SISTEMA IoT Gateway ===");

  // Инициализация дисплея
  display.init();
  display.setScreen(SCREEN_DASHBOARD);
  display.setStatus(false);
  display.updateNodeCount(0, 0);

  // Wi-Fi
  WiFi.softAP(ssid_ap, password_ap);
  Serial.print("✅ AP: ");
  Serial.println(WiFi.softAPIP());

  // Веб-сервер
  server.on("/data", HTTP_POST, []()
            {
        if (server.hasArg("nombre") && server.hasArg("temperatura") && server.hasArg("humedad")) {
            String sensorName = server.arg("nombre");
            float temp = server.arg("temperatura").toFloat();
            float hum = server.arg("humedad").toFloat();
            
            lastActivityTime = millis();
            
            // Обновляем дисплей
            display.setStatus(true);
            display.updateSensorData(sensorName, temp, hum, activeNodeCount);
            display.updateNodeCount(activeNodeCount + 1, sleepNodeCount);
            display.updateLastSeen(0);
            
            Serial.printf("📊 %s | T: %.1f | H: %.1f\n", sensorName.c_str(), temp, hum);
            server.send(200, "text/plain", "OK");
        } else {
            server.send(400, "text/plain", "ERROR");
        } });

  server.begin();
  Serial.println("🚀 Server running");
}

void loop()
{
  server.handleClient();

  // Проверяем таймауты
  if (millis() - lastActivityTime > NODE_TIMEOUT && activeNodeCount > 0)
  {
    // Узел ушел в сон
    activeNodeCount--;
    sleepNodeCount++;
    display.updateNodeCount(activeNodeCount, sleepNodeCount);
    lastActivityTime = millis(); // Сбрасываем таймер
  }

  // Обновляем время последнего обновления
  static unsigned long lastDisplayUpdate = 0;
  if (millis() - lastDisplayUpdate > 10000)
  {
    display.updateLastSeen((millis() - lastActivityTime) / 1000);
    lastDisplayUpdate = millis();
  }

  delay(100);
}