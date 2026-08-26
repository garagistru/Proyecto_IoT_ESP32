// src/network/ED_WebServer.cpp
#include "ED_WebServer.h"
#include "ED_DataManager.h"
#include "../display/ED_Display.h" // ← ДОБАВЛЯЕМ!

extern ED_Display display; // ← ТЕПЕРЬ ВИДИТ
extern ED_DataManager dataManager;
extern unsigned long lastReceiveTime;

// ============================================
// КОНСТРУКТОР
// ============================================
ED_WebServer::ED_WebServer() : server(80) {}

// ============================================
// ЗАПУСК ВЕБ-СЕРВЕРА
// ============================================
void ED_WebServer::begin(const char *ssid, const char *password)
{
    WiFi.softAP(ssid, password);
    delay(100);

    server.on("/api/nodes", HTTP_GET, handleNodes);
    server.on("/api/data", HTTP_GET, handleApiData);
    server.on("/register", HTTP_POST, handleRegister);
    server.on("/data", HTTP_POST, handleSensorData);
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    server.begin();

    Serial.print("✅ WebServer iniciado en ");
    Serial.println(WiFi.softAPIP());
}

// ============================================
// ОБРАБОТЧИК: РЕГИСТРАЦИЯ ДАТЧИКА
// ============================================
void ED_WebServer::handleRegister(AsyncWebServerRequest *request)
{
    if (request->hasParam("nombre", true))
    {
        String nombre = request->getParam("nombre", true)->value();
        dataManager.registerNode(nombre);

        lastReceiveTime = millis();

        request->send(200, "text/plain", "OK");
        display.drawRealTimeData();
    }
    else
    {
        request->send(400, "text/plain", "Bad Request");
    }
}

// ============================================
// ОБРАБОТЧИК: ПОЛУЧЕНИЕ ДАННЫХ ОТ ДАТЧИКА
// ============================================
void ED_WebServer::handleSensorData(AsyncWebServerRequest *request)
{
    if (request->hasParam("nombre", true) &&
        request->hasParam("temperatura", true) &&
        request->hasParam("humedad", true))
    {

        String nombre = request->getParam("nombre", true)->value();
        float temp = request->getParam("temperatura", true)->value().toFloat();
        float hum = request->getParam("humedad", true)->value().toFloat();

        dataManager.onNewSensorData(nombre, temp, hum);

        lastReceiveTime = millis();

        request->send(200, "text/plain", "OK");
        display.drawRealTimeData();
    }
    else
    {
        request->send(400, "text/plain", "Bad Request");
    }
}

// ============================================
// ОБРАБОТЧИК: API /api/nodes
// ============================================
void ED_WebServer::handleNodes(AsyncWebServerRequest *request)
{
    JsonDocument doc;
    doc["total"] = sysState.totalNodes;
    doc["active"] = sysState.activeNodes;
    doc["dormant"] = sysState.dormantNodes;
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

// ============================================
// ОБРАБОТЧИК: API /api/data
// ============================================
void ED_WebServer::handleApiData(AsyncWebServerRequest *request)
{
    JsonDocument doc;
    doc["receive"] = sysState.lastReceive;
    doc["transmit"] = sysState.lastTransmit;
    doc["buffer"] = sysState.bufferSize;
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}