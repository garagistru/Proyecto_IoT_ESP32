// src/display/ED_State.h
#ifndef ED_STATE_H
#define ED_STATE_H

#include <Arduino.h>
#include <String.h>

// Структура, хранящая все данные для отображения на экране
struct DisplayState
{
    // Сеть и узлы
    int totalNodes = 0;
    int activeNodes = 0;
    int dormantNodes = 0;

    // Временные метки
    String lastReceive = "Nunca";
    String lastTransmit = "Nunca";

    // Данные с датчиков (задел на будущее)
    float currentTemp = 0.0;
    float currentHum = 0.0;

    // Статусы
    bool isServerConnected = false;
};

// Глобальный экземпляр состояния (будет обновляться в main.cpp)
extern DisplayState sysState;

#endif