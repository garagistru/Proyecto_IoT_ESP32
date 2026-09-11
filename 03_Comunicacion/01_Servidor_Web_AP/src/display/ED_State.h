#ifndef ED_STATE_H
#define ED_STATE_H

#include <Arduino.h>
#include <vector>

struct SensorNode
{
    String name;   // Имя станка (главный ключ)
    String mac;    // MAC-адрес датчика (для истории)
    String status; // "online", "offline", "abrir", "cerrado"
    int actions;
    unsigned long lastSeen;
    bool isActive;
};

struct DisplayState
{
    bool isConnected = false;
    int totalNodes = 0;
    int activeNodes = 0;
    int dormantNodes = 0;
    String lastReceive = "Nunca";
    String lastTransmit = "Nunca";
    int bufferSize = 0;
    std::vector<SensorNode> nodes;
    String version = "v1.2.0";
};

extern DisplayState sysState;

#endif