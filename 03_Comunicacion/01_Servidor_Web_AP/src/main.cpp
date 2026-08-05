#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "display/display_manager.h"

const char *ssid_ap = "Mechanic";
const char *password_ap = "12345678";
WebServer server(80);

// РЕАЛЬНЫЕ СЧЕТЧИКИ
int totalNodes = 0;
int activeNodes = 0;
int sleepNodes = 0;
unsigned long lastDataTime = 0;
unsigned long systemStartTime = 0;

String activeSensors[10];
int sensorCount = 0;

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
  display.updateMetrics(0, 0, 0);
  display.updateTime(0, 0);

  // 2. Wi-Fi
  WiFi.softAP(ssid_ap, password_ap);
  Serial.print("✅ AP: ");
  Serial.println(WiFi.softAPIP());

  display.setNetworkStatus(true);
  systemStartTime = millis();

  // 3. Веб-сервер
  server.on("/", HTTP_GET, []()
            { server.send(200, "text/plain", "ARM Gateway OK"); });

  server.on("/data", HTTP_POST, []()
            {
        if (server.hasArg("nombre") && server.hasArg("temperatura") && server.hasArg("humedad")) {
            String sensorName = server.arg("nombre");
            float temp = server.arg("temperatura").toFloat();
            float hum = server.arg("humedad").toFloat();
            
            lastDataTime = millis();
            
            bool found = false;
            for (int i = 0; i < sensorCount; i++) {
                if (activeSensors[i] == sensorName) {
                    found = true;
                    break;
                }
            }
            
            if (!found && sensorCount < 10) {
                activeSensors[sensorCount] = sensorName;
                sensorCount++;
                totalNodes = sensorCount;
                activeNodes = sensorCount;
                sleepNodes = 0;
            }
            
            display.updateMetrics(totalNodes, activeNodes, sleepNodes);
            
            Serial.printf(" %s | T: %.1f | H: %.1f | Total: %d, Active: %d\n", 
                         sensorName.c_str(), temp, hum, totalNodes, activeNodes);
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

  // Обновляем время каждую секунду
  static unsigned long lastTimeUpdate = 0;
  if (millis() - lastTimeUpdate > 1000)
  {
    unsigned long lastDataSec = (lastDataTime > 0) ? (lastDataTime / 1000) : 0;
    unsigned long currentSec = (millis() - systemStartTime) / 1000;
    display.updateTime(lastDataSec, currentSec);
    lastTimeUpdate = millis();
  }

  delay(100);
}