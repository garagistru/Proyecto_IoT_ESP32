#include "ED_ServerLink.h"
#include "../ED_Utils.h"

extern ED_DataManager dataManager;

ED_ServerLink::ED_ServerLink() {}

void ED_ServerLink::begin()
{
    LOGLN("🔗 ServerLink готов (JSON transit)");
}

bool ED_ServerLink::update()
{
    DataPacket packet;
    if (!dataManager.getNextPendingPacket(packet))
        return false;

    if (sendPacket(packet))
    {
        if (waitForAck())
        {
            dataManager.markPacketAsSent(packet.timestamp);
            return true;
        }
    }
    return false;
}

// ⚠️ ВАЖНО: JSON уходит в Serial (USB-OTG → Ubuntu). НЕ менять на LOG!
bool ED_ServerLink::sendPacket(const DataPacket &packet)
{
    String json = "{\"name\":\"" + packet.name + "\",";
    json += "\"mac\":\"" + packet.mac + "\",";
    json += "\"raw\":\"" + packet.rawBody + "\"}";

    Serial.println(json);
    return true;
}

// ⚠️ ВАЖНО: ACK читаем из Serial (USB-OTG → Ubuntu). НЕ менять на Serial0!
bool ED_ServerLink::waitForAck()
{
    unsigned long start = millis();
    while (millis() - start < 1000)
    {
        if (Serial.available())
        {
            String response = Serial.readStringUntil('\n');
            response.trim();
            if (response == "OK")
                return true;
        }
        delay(10);
    }
    return false;
}

int ED_ServerLink::getPendingCount() const
{
    return dataManager.getBufferSize();
}