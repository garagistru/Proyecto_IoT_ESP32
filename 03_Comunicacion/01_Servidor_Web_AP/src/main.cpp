#include <Arduino.h>
#include <LittleFS.h>

#include "display/ED_State.h"
#include "display/ED_Display.h"
#include "network/ED_WebServer.h"
#include "network/ED_DataManager.h"
#include "network/ED_ServerLink.h"

ED_Display display;
ED_WebServer webServer;
ED_DataManager dataManager;
ED_ServerLink serverLink;

unsigned long lastReceiveTime = 0;
unsigned long lastTransmitTime = 0;

String formatTimeAgo(unsigned long ts)
{
    if (ts == 0)
        return "Nunca";
    unsigned long e = (millis() - ts) / 1000;
    if (e < 60)
        return "hace " + String(e) + "s";
    if (e < 3600)
        return "hace " + String(e / 60) + "m";
    return "hace " + String(e / 3600) + "h";
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n============================================");
    Serial.println("     ESP32-S3 ГОЛОВНОЕ УСТРОЙСТВО");
    Serial.println("============================================");

    display.begin();
    display.setBrightness(200);

    if (!LittleFS.begin(true))
    {
        Serial.println("❌ LittleFS error");
    }

    // ⬅️ syncTime() УБРАНА — NTP не работает в AP-режиме
    // Время: uptime (от запуска)

    webServer.begin("Mechanic", "12345678");
    serverLink.begin();

    display.drawRealTimeData();
    Serial.println("✅ Система готова (время: uptime)");
}

void loop()
{
    webServer.update();

    // Отправка данных на сервер (каждые 5 сек)
    static unsigned long lastSend = 0;
    if (millis() - lastSend > 5000)
    {
        lastSend = millis();
        serverLink.update();
        lastTransmitTime = millis();
    }

    // Проверка таймаута датчиков (каждые 10 сек)
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 10000)
    {
        lastCheck = millis();
        dataManager.checkNodeTimeout();
        display.drawRealTimeData();
    }

    // Обновление таймеров
    static unsigned long lastTimer = 0;
    if (millis() - lastTimer > 1000)
    {
        lastTimer = millis();
        sysState.lastReceive = formatTimeAgo(lastReceiveTime);
        sysState.lastTransmit = formatTimeAgo(lastTransmitTime);
        display.drawRealTimeData();
    }

    delay(10);
}