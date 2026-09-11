#ifndef ED_DATAMANAGER_H
#define ED_DATAMANAGER_H

#include <Arduino.h>
#include <vector>
#include "../display/ED_State.h"

#define MAX_BUFFER_SIZE 20
#define NODE_TIMEOUT 300000 // 5 минут

struct DataPacket
{
    String type; // "shift"
    String name; // Имя станка
    String mac;  // MAC датчика
    int actions;
    String startTime;
    String endTime;
    String timeSource;
    unsigned long timestamp;
    bool isSent;
};

class ED_DataManager
{
public:
    ED_DataManager();

    void registerNode(const String &name, const String &mac);
    void updateNode(const String &name, const String &mac, int actions,
                    const String &start, const String &end, const String &timeSource);
    void setNodeStatus(const String &name, const String &status);
    void checkNodeTimeout();

    bool getNextPendingPacket(DataPacket &outPacket);
    void markPacketAsSent(const String &name, unsigned long timestamp);
    void cleanUp();

    String getDevicesJson();

    int getTotalNodes() const { return sysState.totalNodes; }
    int getActiveNodes() const { return sysState.activeNodes; }
    int getDormantNodes() const { return sysState.dormantNodes; }
    int getBufferSize() const { return sysState.bufferSize; }

private:
    std::vector<DataPacket> dataBuffer;
    int findNodeIndexByName(const String &name);
};

extern DisplayState sysState;

#endif