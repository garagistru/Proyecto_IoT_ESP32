// src/network/ED_DataManager.h
#ifndef ED_DATAMANAGER_H
#define ED_DATAMANAGER_H

#include <Arduino.h>
#include <vector>
#include <algorithm>
#include "../display/ED_State.h"

// ============================================
// КОНСТАНТЫ
// ============================================
#define MAX_BUFFER_SIZE 20
#define NODE_TIMEOUT 3600000 // 1 час

// ============================================
// СТРУКТУРА ПАКЕТА
// ============================================
struct SensorDataPacket
{
    String sensorName;
    float temp;
    float hum;
    unsigned long timestamp;
    bool isSent;
};

// ============================================
// КЛАСС
// ============================================
class ED_DataManager
{
public:
    ED_DataManager();

    void registerNode(const String &name);
    void onNewSensorData(const String &name, float temp, float hum);
    bool getNextPendingPacket(SensorDataPacket &outPacket);
    void markPacketAsSent(const String &sensorName);
    void cleanUp();
    void checkNodeTimeout();
    void updateLastTransmit();

private:
    std::vector<SensorDataPacket> dataBuffer;
    int findNodeIndex(const String &name);
};

extern DisplayState sysState;

#endif