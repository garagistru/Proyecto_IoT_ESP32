#ifndef ED_WEBSERVER_H
#define ED_WEBSERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <time.h>
#include "ED_DataManager.h"
#include "../display/ED_Display.h"

extern ED_DataManager dataManager;
extern ED_Display display;

class ED_WebServer
{
public:
    ED_WebServer();
    void begin(const char *ssid, const char *password);
    void update();

private:
    AsyncWebServer server;

    static void handleTime(AsyncWebServerRequest *request);
    static void handleData(AsyncWebServerRequest *request);
    static void handleStatus(AsyncWebServerRequest *request);
    static void handleDevices(AsyncWebServerRequest *request);

    static String getTimeJson();
};

#endif