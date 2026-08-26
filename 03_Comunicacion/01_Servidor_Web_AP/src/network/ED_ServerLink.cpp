// src/network/ED_ServerLink.cpp
#include "ED_ServerLink.h"
#include "ED_DataManager.h"

extern ED_DataManager dataManager;
extern unsigned long lastTransmitTime; // ← ВРЕМЯ ОТПРАВКИ

// ============================================
// КОНСТРУКТОР
// ============================================
ED_ServerLink::ED_ServerLink() {}

// ============================================
// ИНИЦИАЛИЗАЦИЯ
// ============================================
void ED_ServerLink::begin()
{
    Serial.println("🔗 Servidor Link iniciado");
    Serial.println("{\"test\":\"hello_ubuntu\"}");
}

// ============================================
// ОБНОВЛЕНИЕ (ВЫЗЫВАЕТСЯ ИЗ LOOP)
// ============================================
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

            // ===== ФИКСИРУЕМ ВРЕМЯ ОТПРАВКИ =====
            lastTransmitTime = millis();
        }
    }
}

// ============================================
// ОЖИДАНИЕ ПОДТВЕРЖДЕНИЯ ОТ СЕРВЕРА
// ============================================
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