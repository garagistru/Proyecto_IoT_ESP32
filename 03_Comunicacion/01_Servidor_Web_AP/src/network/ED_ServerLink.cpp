#include "ED_ServerLink.h"

extern ED_DataManager dataManager;

ED_ServerLink::ED_ServerLink() {}

void ED_ServerLink::begin() {
    Serial.println("🔗 ServerLink готов");
}

void ED_ServerLink::update() {
    DataPacket packet;
    
    if (!dataManager.getNextPendingPacket(packet)) {
        return;  // Нет данных для отправки
    }
    
    if (sendPacket(packet)) {
        if (waitForAck()) {
            dataManager.markPacketAsSent(packet.name, packet.timestamp);
        }
    }
}

bool ED_ServerLink::sendPacket(const DataPacket &packet) {
    // ⬅️ Формат JSON — единый для всех типов
    String json = "{\"type\":\"" + packet.type + "\"";
    json += ",\"name\":\"" + packet.name + "\"";
    json += ",\"mac\":\"" + packet.mac + "\"";
    json += ",\"actions\":" + String(packet.actions);
    json += ",\"start\":\"" + packet.startTime + "\"";
    json += ",\"end\":\"" + packet.endTime + "\"";
    json += ",\"time_source\":\"" + packet.timeSource + "\"";
    json += "}";
    
    Serial.println(json);  // Отправка через USB-OTG
    return true;
}

bool ED_ServerLink::waitForAck() {
    unsigned long start = millis();
    while (millis() - start < 1000) {
        if (Serial.available()) {
            String response = Serial.readStringUntil('\n');
            response.trim();
            if (response == "OK") {
                return true;
            }
        }
        delay(10);
    }
    return false;
}

int ED_ServerLink::getPendingCount() const {
    return dataManager.getBufferSize();
}