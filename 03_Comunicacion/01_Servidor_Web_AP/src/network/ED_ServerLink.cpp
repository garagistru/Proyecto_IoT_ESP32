// src/network/ED_ServerLink.cpp
#include "ED_ServerLink.h"
#include "ED_DataManager.h"

// Сообщаем компилятору, что dataManager определен в main.cpp
extern ED_DataManager dataManager;

ED_ServerLink::ED_ServerLink() {}

void ED_ServerLink::begin()
{
    // Глобальный Serial уже инициализирован в main.cpp
    // Отправляем тестовое сообщение
    Serial.println("{\"test\":\"hello_ubuntu\"}");
}

void ED_ServerLink::update()
{
    SensorDataPacket packet;

    // Проверяем, есть ли в буфере неотправленные данные
    if (dataManager.getNextPendingPacket(packet))
    {
        // Формируем JSON
        String json = "{\"sensor\":\"" + packet.sensorName +
                      "\",\"temp\":" + String(packet.temp, 1) +
                      ",\"hum\":" + String(packet.hum, 1) + "}";

        // Отправляем на сервер через Serial (COM6)
        Serial.println(json);

        // Ждем подтверждение
        if (waitForAck())
        {
            dataManager.markPacketAsSent(packet.sensorName);
        }
    }
}

bool ED_ServerLink::waitForAck()
{
    unsigned long startTime = millis();
    while (millis() - startTime < 500)
    {
        if (Serial.available())
        {
            String response = Serial.readStringUntil('\n');
            response.trim();
            if (response == "OK")
            {
                return true;
            }
        }
    }
    return false;
}