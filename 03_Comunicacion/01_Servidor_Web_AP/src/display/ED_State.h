#ifndef ED_STATE_H
#define ED_STATE_H

#include <Arduino.h>
#include <vector>

struct SensorNode
{
    String name;             // Уникальный ключ
    String mac;              // MAC датчика
    String status;           // "abrir" / "cerrado"
    int actions;             // Последнее кол-во действий
    unsigned long lastSeen;  // millis() последнего контакта
    unsigned long firstSeen; // millis() первого появления
    bool isActive;           // (millis - lastSeen) < dormant_threshold
    bool isInAP;             // MAC в списке Wi-Fi станций
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