#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "display/display_manager.h"

const char *ssid_ap = "Mechanic";
const char *password_ap = "12345678";
WebServer server(80);

// Состояние системы
int totalNodes = 29;                       // Для демо
int activeNodes = 24;                      // Для демо
int sleepNodes = 5;                        // Для демо
unsigned long systemUptimeSeconds = 52385; // ~14:33:05 для демо

void setup()
{
  Serial.begin(115200);
  delay(3000);

  Serial.println("========================================");
  Serial.println(PROJECT_NAME);
  Serial.println(DEVICE_FULL_NAME);
  Serial.println("========================================");

  // 1. Дисплей
  display.init();
  display.setNetworkStatus(false);
  display.updateMetrics(totalNodes, activeNodes, sleepNodes);
  display.updateTime(systemUptimeSeconds);

  // 2. Wi-Fi
  WiFi.softAP(ssid_ap, password_ap);
  Serial.print("✅ AP: ");
  Serial.println(WiFi.softAPIP());

  display.setNetworkStatus(true); // Сеть поднята

  // 3. Веб-сервер
  server.on("/", HTTP_GET, []()
            { server.send(200, "text/plain", "ARM Gateway OK"); });

  server.on("/data", HTTP_POST, []()
            {
        if (server.hasArg("nombre") && server.hasArg("temperatura") && server.hasArg("humedad")) {
            String sensorName = server.arg("nombre");
            float temp = server.arg("temperatura").toFloat();
            float hum = server.arg("humedad").toFloat();
            
            // Имитация получения данных: обновляем время и немного меняем метрики для теста
            systemUptimeSeconds = (systemUptimeSeconds / 60) * 60; // Сброс секунд для наглядности
            activeNodes = 24; 
            
            display.updateMetrics(totalNodes, activeNodes, sleepNodes);
            display.updateTime(systemUptimeSeconds);
            
            Serial.printf("📊 %s | T: %.1f | H: %.1f\n", sensorName.c_str(), temp, hum);
            server.send(200, "text/plain", "OK: " + sensorName);
        } else {
            server.send(400, "text/plain", "ERROR");
        } });

  server.begin();
  Serial.println("🚀 HTTP Server running");
}

void loop()
{
  server.handleClient();

  // Демонстрация тикания времени (каждую секунду)
  static unsigned long lastTick = 0;
  if (millis() - lastTick > 1000)
  {
    systemUptimeSeconds++;
    display.updateTime(systemUptimeSeconds);
    lastTick = millis();
  }

  delay(10);
}