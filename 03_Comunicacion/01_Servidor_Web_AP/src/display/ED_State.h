// src/display/ED_State.h
#ifndef ED_STATE_H
#define ED_STATE_H

#include <Arduino.h>
#include <vector>

struct SensorNode
{
    String name;
    unsigned long lastSeen;
    bool isActive;
};

struct DisplayState
{
    // ----- Статус сети -----
    bool isConnected = false;

    // ----- Статистика узлов -----
    int totalNodes = 0;
    int activeNodes = 0;
    int dormantNodes = 0;

    // ----- Время передачи (ОБНОВЛЯЕТСЯ ИЗ main.cpp) -----
    String lastReceive = "Nunca";
    String lastTransmit = "Nunca";

    // ----- Буфер -----
    int bufferSize = 0;

    // ----- Список датчиков -----
    std::vector<SensorNode> nodes;

    // ----- Версия -----
    String version = "v1.2.0";
};

extern DisplayState sysState;

#endif