// src/main.cpp
#include <Arduino.h>
#include <LittleFS.h>

#include "display/ED_State.h"
#include "display/ED_Display.h"
#include "network/ED_WebServer.h"
#include "network/ED_DataManager.h"
#include "network/ED_ServerLink.h"

// Глобальные объекты
ED_Display display;
ED_WebServer webServer;
ED_DataManager dataManager;
ED_ServerLink serverLink;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("ED_BOOT_OK");

    // --- ТЕСТОВАЯ ОТПРАВКА ДАННЫХ НА СЕРВЕР (COM6) ---
    Serial.println("{\"test\":\"hello_ubuntu\",\"sensor\":\"microclima_1\",\"temp\":25.5,\"hum\":60.0}");

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

void loop()
{

    // --- ОТПРАВКА ДАННЫХ НА СЕРВЕР ---
    static unsigned long lastSend = 0;
    if (millis() - lastSend > 5000)
    {
        lastSend = millis();
        serverLink.update();
    }

    // --- ПРОВЕРКА СПЯЩИХ ДАТЧИКОВ ---
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 10000)
    {
        lastCheck = millis();
        dataManager.checkNodeTimeout();
        display.drawRealTimeData();
    }

    delay(100);
}