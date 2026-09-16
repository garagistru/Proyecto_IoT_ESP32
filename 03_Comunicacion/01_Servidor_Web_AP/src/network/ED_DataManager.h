#ifndef ED_DATAMANAGER_H
#define ED_DATAMANAGER_H

#include <Arduino.h>
#include <vector>
#include "../display/ED_State.h"

#define MAX_BUFFER_SIZE 30

// Упрощённый пакет — только rawBody + метаданные
struct DataPacket
{
    String rawBody;          // Вся строка: mac=...&name=...&...&boot_id=...
    String name;             // Извлечено для логов
    String mac;              // Извлечено для логов
    unsigned long timestamp; // millis() получения
    bool isSent;
};

class ED_DataManager
{
public:
    ED_DataManager();

    // Регистрация
    void registerNode(const String &name, const String &mac);
    void setNodeStatus(const String &name, const String &status);
    void updateAPStatus(); // Сверка с Wi-Fi станциями
    void checkNodeTimeout(unsigned long dormantThreshold);

    // Очередь
    void queueRawPacket(const String &rawBody, const String &name, const String &mac);
    bool getNextPendingPacket(DataPacket &outPacket);
    void markPacketAsSent(unsigned long timestamp);
    void cleanUp();

    // Статистика
    String getDevicesArrayJson(); // ← заменить getDevicesJson
    int getTotalNodes() const { return sysState.totalNodes; }
    int getActiveNodes() const { return sysState.activeNodes; }
    int getDormantNodes() const { return sysState.dormantNodes; }
    int getBufferSize() const { return sysState.bufferSize; }

private:
    std::vector<DataPacket> dataBuffer;
    int findNodeIndexByName(const String &name);
    int findNodeIndexByMac(const String &mac);
};

extern DisplayState sysState;

#endif