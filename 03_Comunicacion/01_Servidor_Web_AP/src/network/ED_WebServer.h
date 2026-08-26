#ifndef ED_WEBSERVER_H
#define ED_WEBSERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

class ED_WebServer
{
public:
    ED_WebServer();
    void begin(const char *ssid, const char *password);

private:
    AsyncWebServer server;

    // Обработчики маршрутов
    static void handleNodes(AsyncWebServerRequest *request);
    static void handleApiData(AsyncWebServerRequest *request);
    static void handleRegister(AsyncWebServerRequest *request);
    static void handleSensorData(AsyncWebServerRequest *request);
    static void handleStatus(AsyncWebServerRequest *request); // <-- НОВЫЙ
};

#endif