#ifndef ED_STATE_H
#define ED_STATE_H

#include <Arduino.h>
#include <String.h>

struct DisplayState
{
    // Сеть и узлы
    int totalNodes = 0;
    int activeNodes = 0;
    int dormantNodes = 0;

    // Временные метки
    String lastReceive = "Nunca";
    String lastTransmit = "Nunca";

    // Данные с датчиков
    float currentTemp = 0.0;
    float currentHum = 0.0;

    // Статусы
    bool isServerConnected = false;

    // Буфер передачи
    int bufferSize = 0;
};

// Объявляем внешнюю переменную
extern DisplayState sysState;

#endif