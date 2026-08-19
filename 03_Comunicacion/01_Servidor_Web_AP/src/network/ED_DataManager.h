// src/network/ED_DataManager.h
#ifndef ED_DATAMANAGER_H
#define ED_DATAMANAGER_H

#include <Arduino.h>
#include <vector>
#include <algorithm>
#include "../display/ED_State.h"

#define MAX_BUFFER_SIZE 20
#define NODE_TIMEOUT 3600000 // 30 segundos sin datos → dormido

struct SensorDataPacket
{
    String sensorName;
    float temp;
    float hum;
    unsigned long timestamp;
    bool isSent;
};

class ED_DataManager
{
public:
    ED_DataManager();

    // --- Регистрация нового датчика ---
    void registerNode(const String &name);

    // --- Получение данных от датчика ---
    void onNewSensorData(const String &name, float temp, float hum);

    // --- Работа с буфером ---
    bool getNextPendingPacket(SensorDataPacket &outPacket);
    void markPacketAsSent(const String &sensorName);
    void cleanUp();

    // --- Проверка таймаута (спящие датчики) ---
    void checkNodeTimeout();

    // --- Обновление времени последней отправки ---
    void updateLastTransmit();

    // --- Получение количества узлов ---
    int getTotalNodes() const { return sysState.totalNodes; }
    int getActiveNodes() const { return sysState.activeNodes; }
    int getDormantNodes() const { return sysState.dormantNodes; }
    int getBufferSize() const { return sysState.bufferSize; }

private:
    std::vector<SensorDataPacket> dataBuffer;

    // Поиск датчика по имени
    int findNodeIndex(const String &name);
};

#endif