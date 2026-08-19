// src/network/ED_WebServer.h
#ifndef ED_WEBSERVER_H
#define ED_WEBSERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

// Подключаем наше единое состояние, чтобы обновлять данные
#include "../display/ED_State.h"
// Подключаем дисплей, чтобы вызывать перерисовку при получении данных
#include "../display/ED_Display.h"

class ED_WebServer {
public:
    ED_WebServer();
    
    // Метод для запуска Wi-Fi точки доступа и веб-сервера
    void begin(const char* ssid, const char* password);

private:
    AsyncWebServer server; // Сервер на 80 порту
    
    // Статические обработчики запросов (обязательно static, чтобы передать их в server.on)
    static void handleNodes(AsyncWebServerRequest *request);
    static void handleApiData(AsyncWebServerRequest *request);
    static void handleRegister(AsyncWebServerRequest *request);
    static void handleSensorData(AsyncWebServerRequest *request);
};

#endif