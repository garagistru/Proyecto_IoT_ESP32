#ifndef ED_SERVERLINK_H
#define ED_SERVERLINK_H

#include <Arduino.h>
#include "ED_DataManager.h"

class ED_ServerLink
{
public:
    ED_ServerLink();
    void begin();
    bool update(); // ← было void
    int getPendingCount() const;

private:
    bool sendPacket(const DataPacket &packet);
    bool waitForAck();
};

#endif