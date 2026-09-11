#include "ED_DataManager.h"

DisplayState sysState;

ED_DataManager::ED_DataManager()
{
    dataBuffer.reserve(MAX_BUFFER_SIZE);
}

int ED_DataManager::findNodeIndexByName(const String &name)
{
    for (size_t i = 0; i < sysState.nodes.size(); i++)
    {
        if (sysState.nodes[i].name == name)
            return i;
    }
    return -1;
}

void ED_DataManager::registerNode(const String &name, const String &mac)
{
    int idx = findNodeIndexByName(name);
    if (idx != -1)
    {
        sysState.nodes[idx].mac = mac;
        sysState.nodes[idx].lastSeen = millis();
        sysState.nodes[idx].isActive = true;
        if (sysState.nodes[idx].status == "offline")
        {
            sysState.nodes[idx].status = "online";
        }
        return;
    }

    SensorNode node;
    node.name = name;
    node.mac = mac;
    node.status = "online";
    node.actions = 0;
    node.lastSeen = millis();
    node.isActive = true;

    sysState.nodes.push_back(node);
    sysState.totalNodes++;
    sysState.activeNodes++;
    sysState.lastReceive = "hace 0s";

    Serial.printf("📥 Nuevo nodo: %s (MAC: %s)\n", name.c_str(), mac.c_str());
    Serial.printf("   Total: %d\n", sysState.totalNodes);
}

void ED_DataManager::updateNode(const String &name, const String &mac, int actions,
                                const String &start, const String &end, const String &timeSource)
{
    int idx = findNodeIndexByName(name);
    if (idx == -1)
    {
        registerNode(name, mac);
        idx = findNodeIndexByName(name);
    }

    if (idx != -1)
    {
        sysState.nodes[idx].mac = mac;
        sysState.nodes[idx].actions = actions;
        sysState.nodes[idx].lastSeen = millis();
        sysState.nodes[idx].isActive = true;
        sysState.nodes[idx].status = "online";
        sysState.lastReceive = "hace 0s";
    }

    DataPacket packet;
    packet.type = "shift";
    packet.name = name;
    packet.mac = mac;
    packet.actions = actions;
    packet.startTime = start;
    packet.endTime = end;
    packet.timeSource = timeSource;
    packet.timestamp = millis();
    packet.isSent = false;

    if (dataBuffer.size() < MAX_BUFFER_SIZE)
    {
        dataBuffer.push_back(packet);
    }
    else
    {
        for (auto it = dataBuffer.begin(); it != dataBuffer.end(); ++it)
        {
            if (!it->isSent)
            {
                dataBuffer.erase(it);
                break;
            }
        }
        dataBuffer.push_back(packet);
    }

    sysState.bufferSize = dataBuffer.size();
    Serial.printf("📥 Datos de %s: actions=%d, buffer=%d\n",
                  name.c_str(), actions, sysState.bufferSize);
}

void ED_DataManager::setNodeStatus(const String &name, const String &status)
{
    int idx = findNodeIndexByName(name);
    if (idx == -1)
        return;
    sysState.nodes[idx].status = status;
    sysState.nodes[idx].lastSeen = millis();
    sysState.nodes[idx].isActive = true;
}

void ED_DataManager::checkNodeTimeout()
{
    unsigned long now = millis();
    bool changed = false;
    for (auto &node : sysState.nodes)
    {
        if (node.isActive && (now - node.lastSeen > NODE_TIMEOUT))
        {
            node.isActive = false;
            node.status = "offline";
            sysState.activeNodes--;
            sysState.dormantNodes++;
            changed = true;
            Serial.printf("💤 Nodo offline: %s\n", node.name.c_str());
        }
    }
}

bool ED_DataManager::getNextPendingPacket(DataPacket &outPacket)
{
    for (auto &p : dataBuffer)
    {
        if (!p.isSent)
        {
            outPacket = p;
            return true;
        }
    }
    return false;
}

void ED_DataManager::markPacketAsSent(const String &name, unsigned long timestamp)
{
    for (auto &p : dataBuffer)
    {
        if (p.name == name && p.timestamp == timestamp && !p.isSent)
        {
            p.isSent = true;
            sysState.lastTransmit = "hace 0s";
            break;
        }
    }
    cleanUp();
}

void ED_DataManager::cleanUp()
{
    dataBuffer.erase(
        std::remove_if(dataBuffer.begin(), dataBuffer.end(),
                       [](const DataPacket &p)
                       { return p.isSent; }),
        dataBuffer.end());
    sysState.bufferSize = dataBuffer.size();
}

String ED_DataManager::getDevicesJson()
{
    String json = "{";
    json += "\"total\":" + String(sysState.totalNodes) + ",";
    json += "\"active\":" + String(sysState.activeNodes) + ",";
    json += "\"dormant\":" + String(sysState.dormantNodes) + ",";
    json += "\"devices\":[";
    for (size_t i = 0; i < sysState.nodes.size(); i++)
    {
        if (i > 0)
            json += ",";
        json += "{";
        json += "\"name\":\"" + sysState.nodes[i].name + "\",";
        json += "\"mac\":\"" + sysState.nodes[i].mac + "\",";
        json += "\"status\":\"" + sysState.nodes[i].status + "\",";
        json += "\"active\":" + String(sysState.nodes[i].isActive ? "true" : "false");
        json += "}";
    }
    json += "]}";
    return json;
}