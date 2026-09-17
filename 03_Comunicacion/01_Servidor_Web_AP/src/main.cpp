#include <Arduino.h>
#include <LittleFS.h>
#include <Preferences.h>

#include "ED_Utils.h"
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

// Читает команды из Serial0 (COM4 → Windows, USB-UART)
void handleSerialCommands()
{
    if (!Serial0.available())
        return;
    String cmd = Serial0.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() == 0)
        return;

    if (cmd.startsWith("setdormant:"))
    {
        long v = cmd.substring(11).toInt();
        if (v > 0)
        {
            dormantThreshold = (unsigned long)v * 1000;
            prefs.putULong("dormant", (unsigned long)v);
            LOG("OK dormant=%lu sec\n", v);
        }
        else
        {
            LOG("ERR invalid value\n");
        }
    }
    else if (cmd == "info")
    {
        LOG("=== INFO ===\n");
        LOG("Dormant threshold: %lu s\n", dormantThreshold / 1000);
        LOG("Buffer: %d/%d\n", dataManager.getBufferSize(), 30);
        LOG("Total nodes: %d\n", sysState.totalNodes);
        LOG("Active: %d\n", sysState.activeNodes);
        LOG("Dormant: %d\n", sysState.dormantNodes);
        LOG("Last receive: %s\n", sysState.lastReceive.c_str());
        LOG("Last transmit: %s\n", sysState.lastTransmit.c_str());
    }
    else
    {
        LOG("ERR unknown command: %s\n", cmd.c_str());
    }
}

void setup()
{
    // USB-OTG → Ubuntu (данные + ACK)
    Serial.begin(115200);

    // USB-UART (CH343) → Windows (COM4) — логи + команды
    Serial0.begin(115200);

    delay(1000);
    LOGLN("");
    LOGLN(ED_BANNER);

    display.begin();
    display.setBrightness(200);

    if (!LittleFS.begin(true))
        LOGLN("❌ LittleFS error");

    prefs.begin("head_unit", false);
    dormantThreshold = prefs.getULong("dormant", 3600) * 1000;
    LOG("Dormant: %lu s\n", dormantThreshold / 1000);

    webServer.begin("Mechanic", "12345678");
    serverLink.begin();

    display.drawRealTimeData();
    LOGLN("Ready");
}

void loop()
{
    webServer.update();

    // Отправка на сервер (каждые 5 сек)
    static unsigned long lastSend = 0;
    if (millis() - lastSend > 5000)
    {
        lastSend = millis();
        if (serverLink.update())
        {
            lastTransmitTime = millis();
        }
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