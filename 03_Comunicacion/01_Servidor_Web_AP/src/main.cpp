// src/main.cpp
#include <Arduino.h>
#include <LittleFS.h>

#include "display/ED_State.h"
#include "display/ED_Display.h"
#include "network/ED_WebServer.h"
#include "network/ED_DataManager.h"
#include "network/ED_ServerLink.h"

// ============================================
// ГЛОБАЛЬНЫЕ ОБЪЕКТЫ
// ============================================
ED_Display display;
ED_WebServer webServer;
ED_DataManager dataManager;
ED_ServerLink serverLink;

// ============================================
// ВРЕМЕННЫЕ МЕТКИ ДЛЯ ТАЙМЕРОВ
// ============================================
unsigned long lastReceiveTime = 0;  // Время последнего получения данных от датчика
unsigned long lastTransmitTime = 0; // Время последней успешной отправки на сервер

// ============================================
// ФУНКЦИЯ ФОРМАТИРОВАНИЯ ВРЕМЕНИ
// ============================================
String formatTimeAgo(unsigned long timestamp)
{
    if (timestamp == 0)
        return "Nunca";

    unsigned long elapsed = (millis() - timestamp) / 1000;
    if (elapsed < 60)
    {
        return "hace " + String(elapsed) + "s";
    }
    else if (elapsed < 3600)
    {
        unsigned long m = elapsed / 60;
        return "hace " + String(m) + "m";
    }
    else
    {
        unsigned long h = elapsed / 3600;
        return "hace " + String(h) + "h";
    }
}

// ============================================
// SETUP
// ============================================
void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("ED_BOOT_OK");

    // --- Дисплей ---
    display.begin();
    display.setBrightness(200);

    // --- Файловая система ---
    if (!LittleFS.begin(true))
    {
        Serial.println("❌ LittleFS error, restarting...");
        ESP.restart();
    }

    // --- Веб-сервер (Wi-Fi AP) ---
    webServer.begin("Mechanic", "12345678");

    // --- Связь с сервером Ubuntu ---
    serverLink.begin();

    // --- Первая отрисовка дисплея ---
    display.drawRealTimeData();

    Serial.println("✅ Sistema iniciada correctamente");
}

// ============================================
// LOOP
// ============================================
void loop()
{
    // --- 1. ОТПРАВКА ДАННЫХ НА СЕРВЕР (каждые 5 секунд) ---
    static unsigned long lastSend = 0;
    if (millis() - lastSend > 5000)
    {
        lastSend = millis();
        serverLink.update();
    }

    // --- 2. ПРОВЕРКА СПЯЩИХ ДАТЧИКОВ (каждые 10 секунд) ---
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 10000)
    {
        lastCheck = millis();
        dataManager.checkNodeTimeout();
        display.drawRealTimeData();
    }

    // --- 3. ОБНОВЛЕНИЕ ТАЙМЕРОВ КАЖДУЮ СЕКУНДУ ---
    static unsigned long lastTimeUpdate = 0;
    if (millis() - lastTimeUpdate > 1000)
    {
        lastTimeUpdate = millis();

        // Обновляем строки времени в глобальном состоянии
        sysState.lastReceive = formatTimeAgo(lastReceiveTime);
        sysState.lastTransmit = formatTimeAgo(lastTransmitTime);

        // Обновляем дисплей (только таймеры)
        display.drawRealTimeData();
    }

    delay(100);
}