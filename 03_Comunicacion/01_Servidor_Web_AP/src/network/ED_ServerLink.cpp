#include "ED_ServerLink.h"

extern ED_DataManager dataManager;

ED_ServerLink::ED_ServerLink() {}

void ED_ServerLink::begin()
{
    Serial.println("🔗 ServerLink готов (JSON transit)");
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

bool ED_ServerLink::sendPacket(const DataPacket &packet)
{
    String json = "{\"name\":\"" + packet.name + "\",";
    json += "\"mac\":\"" + packet.mac + "\",";
    json += "\"raw\":\"" + packet.rawBody + "\"}";

    Serial.println(json);
    return true;
}

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