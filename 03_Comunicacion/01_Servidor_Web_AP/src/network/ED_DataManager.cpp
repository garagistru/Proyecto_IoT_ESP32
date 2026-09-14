#include "ED_DataManager.h"
#include <WiFi.h>
#include "esp_wifi.h"

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

int ED_DataManager::findNodeIndexByMac(const String &mac)
{
    for (size_t i = 0; i < sysState.nodes.size(); i++)
    {
        if (sysState.nodes[i].mac == mac)
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
        return;
    }

    SensorNode node;
    node.name = name;
    node.mac = mac;
    node.status = "";
    node.actions = 0;
    node.lastSeen = millis();
    node.firstSeen = millis();
    node.isActive = true;
    node.isInAP = false; // Определится при updateAPStatus

    sysState.nodes.push_back(node);
    sysState.totalNodes++;
    sysState.activeNodes++;
    sysState.lastReceive = "hace 0s";

    Serial.printf("📥 Nuevo nodo: %s (MAC: %s)\n", name.c_str(), mac.c_str());
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

// Сверка реестра со списком Wi-Fi станций
void ED_DataManager::updateAPStatus()
{
    wifi_sta_list_t staList;
    esp_wifi_ap_get_sta_list(&staList);

    // Сбросить isInAP
    for (auto &node : sysState.nodes)
        node.isInAP = false;

    // Отметить тех, кто в списке
    for (int i = 0; i < staList.num; i++)
    {
        char macStr[13];
        snprintf(macStr, sizeof(macStr), "%02X%02X%02X%02X%02X%02X",
                 staList.sta[i].mac[0], staList.sta[i].mac[1],
                 staList.sta[i].mac[2], staList.sta[i].mac[3],
                 staList.sta[i].mac[4], staList.sta[i].mac[5]);
        int idx = findNodeIndexByMac(String(macStr));
        if (idx != -1)
            sysState.nodes[idx].isInAP = true;
    }
}

// Пересчёт active/dormant с учётом isInAP
void ED_DataManager::checkNodeTimeout(unsigned long dormantThreshold)
{
    unsigned long now = millis();
    int active = 0, dormant = 0, total = 0;

    for (auto &node : sysState.nodes)
    {
        if (!node.isInAP)
            continue; // OFFLINE — скрыт
        total++;

        bool timeout = (now - node.lastSeen) >= dormantThreshold;
        node.isActive = !timeout;

        if (timeout)
            dormant++;
        else
            active++;
    }

    sysState.totalNodes = total;
    sysState.activeNodes = active;
    sysState.dormantNodes = dormant;
}

// Очередь
void ED_DataManager::queueRawPacket(const String &rawBody, const String &name, const String &mac)
{
    DataPacket p;
    p.rawBody = rawBody;
    p.name = name;
    p.mac = mac;
    p.timestamp = millis();
    p.isSent = false;

    if (dataBuffer.size() < MAX_BUFFER_SIZE)
    {
        dataBuffer.push_back(p);
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
        dataBuffer.push_back(p);
    }

    sysState.bufferSize = dataBuffer.size();
    Serial.printf("📦 В очередь: %s (buffer=%d)\n", name.c_str(), sysState.bufferSize);
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

void ED_DataManager::markPacketAsSent(unsigned long timestamp)
{
    for (auto &p : dataBuffer)
    {
        if (p.timestamp == timestamp && !p.isSent)
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
    bool first = true;
    for (auto &node : sysState.nodes)
    {
        if (!node.isInAP)
            continue; // OFFLINE — скрыт
        if (!first)
            json += ",";
        first = false;
        json += "{";
        json += "\"name\":\"" + node.name + "\",";
        json += "\"mac\":\"" + node.mac + "\",";
        json += "\"status\":\"" + node.status + "\",";
        json += "\"active\":" + String(node.isActive ? "true" : "false");
        json += "}";
    }
    json += "]}";
    return json;
}