#include "ED_WebServer.h"
#include "ED_ServerLink.h"

extern ED_DataManager dataManager;
extern ED_Display display;
extern unsigned long lastReceiveTime;

ED_WebServer::ED_WebServer() : server(80) {}

void ED_WebServer::begin(const char *ssid, const char *password)
{
    WiFi.softAP(ssid, password);
    delay(100);

    server.on("/time", HTTP_GET, handleTime);
    server.on("/data", HTTP_POST, handleData);
    server.on("/status", HTTP_POST, handleStatus);
    server.on("/devices", HTTP_GET, handleDevices);
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    server.begin();

    Serial.println("✅ WebServer: " + WiFi.softAPIP().toString());
    Serial.println("   GET  /time");
    Serial.println("   POST /data");
    Serial.println("   POST /status");
    Serial.println("   GET  /devices");
}

void ED_WebServer::update()
{
    // AsyncWebServer работает асинхронно
}

String ED_WebServer::getTimeJson()
{
    // NTP в AP-режиме не работает — используем uptime
    unsigned long s = millis() / 1000;
    int h = (s / 3600) % 24;
    int m = (s % 3600) / 60;
    int sec = s % 60;
    char buf[80];
    snprintf(buf, sizeof(buf),
             "{\"hour\":%d,\"minute\":%d,\"second\":%d,\"full\":\"%02d:%02d:%02d\"}",
             h, m, sec, h, m, sec);
    return String(buf);
}

void ED_WebServer::handleTime(AsyncWebServerRequest *request)
{
    request->send(200, "application/json", getTimeJson());
}

void ED_WebServer::handleData(AsyncWebServerRequest *request)
{
    String mac = request->arg("mac");
    String name = request->arg("name");
    String actions = request->arg("actions");
    String start = request->arg("start");
    String end = request->arg("end");
    String time_source = request->arg("time_source");

    if (name.length() == 0 || actions.length() == 0)
    {
        request->send(400, "application/json",
                      "{\"error\":\"Missing name or actions\"}");
        return;
    }

    Serial.printf("\n📊 Данные от %s (MAC: %s)\n", name.c_str(), mac.c_str());
    Serial.printf("   actions=%s, start=%s, end=%s\n",
                  actions.c_str(), start.c_str(), end.c_str());

    dataManager.updateNode(name, mac, actions.toInt(), start, end, time_source);
    lastReceiveTime = millis();

    request->send(200, "application/json", "{\"status\":\"ok\"}");
    display.drawRealTimeData();
}

void ED_WebServer::handleStatus(AsyncWebServerRequest *request)
{
    String nombre = request->arg("nombre");
    String status = request->arg("status");

    if (nombre.length() == 0 || status.length() == 0)
    {
        request->send(400, "application/json", "{\"error\":\"Missing fields\"}");
        return;
    }

    Serial.printf("📡 %s → %s\n", nombre.c_str(), status.c_str());
    dataManager.setNodeStatus(nombre, status);
    lastReceiveTime = millis();

    request->send(200, "application/json", "{\"status\":\"ok\"}");
    display.drawRealTimeData();
}

void ED_WebServer::handleDevices(AsyncWebServerRequest *request)
{
    request->send(200, "application/json", dataManager.getDevicesJson());
}