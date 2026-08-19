// src/display/ED_State.h
#ifndef ED_STATE_H
#define ED_STATE_H

#include <Arduino.h>
#include <vector>

struct SensorNode {
    String name;
    unsigned long lastSeen;
    bool isActive;
};

struct DisplayState {
    // ----- Статус сети -----
    bool isConnected = false;
    
    // ----- Статистика узлов (только количество) -----
    int totalNodes = 0;
    int activeNodes = 0;
    int dormantNodes = 0;
    
    // ----- Время передачи (только время) -----
    String lastReceive = "---";
    String lastTransmit = "---";
    
    // ----- Буфер (только размер) -----
    int bufferSize = 0;
    
    // ----- Список датчиков (для логики) -----
    std::vector<SensorNode> nodes;
    
    // ----- Версия проекта -----
    String version = "v1.2.0";
};

extern DisplayState sysState;

#endif