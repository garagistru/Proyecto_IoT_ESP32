// src/network/ED_ServerLink.h
#ifndef ED_SERVERLINK_H
#define ED_SERVERLINK_H

#include <Arduino.h>
#include "ED_DataManager.h"

class ED_ServerLink
{
public:
    ED_ServerLink();
    void begin();
    void update(); // Этот метод нужно вызывать в loop()

private:
    bool waitForAck(); // Ожидание подтверждения "OK" от сервера
};

#endif