#include <Arduino.h>
#include <LittleFS.h>
#include <Preferences.h>

#include "display/ED_State.h"
#include "display/ED_Display.h"
#include "network/ED_WebServer.h"
#include "network/ED_DataManager.h"
#include "network/ED_ServerLink.h"

ED_Display display;
ED_WebServer webServer;
ED_DataManager dataManager;
ED_ServerLink serverLink;
Preferences prefs;

unsigned long lastReceiveTime = 0;
unsigned long lastTransmitTime = 0;
unsigned long dormantThreshold = 3600000; // 1 час по умолчанию (мс)

String formatTimeAgo(unsigned long ts)
{
    if (ts == 0)
        return "--";
    unsigned long e = (millis() - ts) / 1000;
    if (e < 60)
        return String(e) + "s"; // ← "16s"
    if (e < 3600)
        return String(e / 60) + "m"; // ← "16m"
    return String(e / 3600) + "h";   // ← "2h"
}

void handleSerialCommands()
{
    if (!Serial.available())
        return;
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd.startsWith("setdormant:"))
    {
        long v = cmd.substring(11).toInt();
        if (v > 0)
        {
            dormantThreshold = (unsigned long)v * 1000;
            prefs.putULong("dormant", (unsigned long)v);
            Serial.printf("OK dormant=%lu sec\n", v);
        }
    }
    else if (cmd == "info")
    {
        Serial.printf("Dormant threshold: %lu s\n", dormantThreshold / 1000);
        Serial.printf("Buffer: %d/%d\n", dataManager.getBufferSize(), 30);
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== ESP32-S3 HEAD UNIT v1.3 ===");

    display.begin();
    display.setBrightness(200);

    if (!LittleFS.begin(true))
        Serial.println("❌ LittleFS error");

    prefs.begin("head_unit", false);
    dormantThreshold = prefs.getULong("dormant", 3600) * 1000;
    Serial.printf("Dormant: %lu s\n", dormantThreshold / 1000);

    webServer.begin("Mechanic", "12345678");
    serverLink.begin();

    display.drawRealTimeData();
    Serial.println("✅ Ready");
}

void loop()
{
    webServer.update();

    // Отправка на сервер (каждые 5 сек)
    static unsigned long lastSend = 0;
    if (millis() - lastSend > 5000)
    {
        lastSend = millis();
        serverLink.update();
        lastTransmitTime = millis();
    }

    // Сверка с Wi-Fi станциями + пересчёт состояний (каждые 10 сек)
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 10000)
    {
        lastCheck = millis();
        dataManager.updateAPStatus();
        dataManager.checkNodeTimeout(dormantThreshold);
        display.drawRealTimeData();
    }

    // Таймеры
    static unsigned long lastTimer = 0;
    if (millis() - lastTimer > 1000)
    {
        lastTimer = millis();
        sysState.lastReceive = formatTimeAgo(lastReceiveTime);
        sysState.lastTransmit = formatTimeAgo(lastTransmitTime);
        display.drawRealTimeData();
    }

    handleSerialCommands();
    delay(10);
}