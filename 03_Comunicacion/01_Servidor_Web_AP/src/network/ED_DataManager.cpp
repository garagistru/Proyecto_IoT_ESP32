// src/network/ED_DataManager.cpp
#include "ED_DataManager.h"

DisplayState sysState;

ED_DataManager::ED_DataManager()
{
    dataBuffer.reserve(MAX_BUFFER_SIZE);
}

// ============================================
// ПОИСК ДАТЧИКА В СПИСКЕ
// ============================================
int ED_DataManager::findNodeIndex(const String &name)
{
    for (size_t i = 0; i < sysState.nodes.size(); i++)
    {
        if (sysState.nodes[i].name == name)
        {
            return i;
        }
    }
    return -1;
}

// ============================================
// РЕГИСТРАЦИЯ ДАТЧИКА
// ============================================
void ED_DataManager::registerNode(const String &name)
{
    int index = findNodeIndex(name);

    if (index == -1)
    {
        SensorNode newNode;
        newNode.name = name;
        newNode.lastSeen = millis();
        newNode.isActive = true;
        sysState.nodes.push_back(newNode);
        sysState.totalNodes++;
        sysState.activeNodes++;
        sysState.lastReceive = "hace 0s";

        Serial.print("📥 Nuevo sensor registrado: ");
        Serial.println(name);
        Serial.print("   Total nodos: ");
        Serial.println(sysState.totalNodes);
    }
    else
    {
        sysState.nodes[index].lastSeen = millis();
        if (!sysState.nodes[index].isActive)
        {
            sysState.nodes[index].isActive = true;
            sysState.activeNodes++;
            sysState.dormantNodes--;
        }
        sysState.lastReceive = "hace 0s";
    }
}

// ============================================
// ПОЛУЧЕНИЕ ДАННЫХ ОТ ДАТЧИКА
// ============================================
void ED_DataManager::onNewSensorData(const String &name, float temp, float hum)
{
    // --- 1. Обновляем статус датчика ---
    sysState.lastReceive = "hace 0s";

    int index = findNodeIndex(name);
    if (index != -1)
    {
        sysState.nodes[index].lastSeen = millis();
        if (!sysState.nodes[index].isActive)
        {
            sysState.nodes[index].isActive = true;
            sysState.activeNodes++;
            sysState.dormantNodes--;
        }
    }
    else
    {
        registerNode(name);
    }

    // --- 2. Добавляем данные в буфер ---
    SensorDataPacket packet;
    packet.sensorName = name;
    packet.temp = temp;
    packet.hum = hum;
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

    Serial.print("📥 Datos de ");
    Serial.print(name);
    Serial.print(": T=");
    Serial.print(temp, 1);
    Serial.print("°C, H=");
    Serial.print(hum, 1);
    Serial.println("%");
    Serial.print("   Buffer: ");
    Serial.println(sysState.bufferSize);
}

// ============================================
// ПОЛУЧЕНИЕ СЛЕДУЮЩЕГО ПАКЕТА ДЛЯ ОТПРАВКИ
// ============================================
bool ED_DataManager::getNextPendingPacket(SensorDataPacket &outPacket)
{
    for (auto &packet : dataBuffer)
    {
        if (!packet.isSent)
        {
            outPacket = packet;
            return true;
        }
    }
    return false;
}

// ============================================
// ПОМЕТКА ПАКЕТА КАК ОТПРАВЛЕННОГО
// ============================================
void ED_DataManager::markPacketAsSent(const String &sensorName)
{
    for (auto &packet : dataBuffer)
    {
        if (packet.sensorName == sensorName && !packet.isSent)
        {
            packet.isSent = true;
            sysState.lastTransmit = "hace 0s";
            break;
        }
    }
    cleanUp();
}

// ============================================
// ОЧИСТКА БУФЕРА ОТ ОТПРАВЛЕННЫХ ПАКЕТОВ
// ============================================
void ED_DataManager::cleanUp()
{
    dataBuffer.erase(
        std::remove_if(dataBuffer.begin(), dataBuffer.end(),
                       [](const SensorDataPacket &p)
                       { return p.isSent; }),
        dataBuffer.end());
    sysState.bufferSize = dataBuffer.size();
}

// ============================================
// ПРОВЕРКА ТАЙМАУТА (СПЯЩИЕ ДАТЧИКИ)
// ============================================
void ED_DataManager::checkNodeTimeout()
{
    unsigned long now = millis();
    bool changed = false;

    for (auto &node : sysState.nodes)
    {
        if (node.isActive && (now - node.lastSeen > NODE_TIMEOUT))
        {
            node.isActive = false;
            sysState.activeNodes--;
            sysState.dormantNodes++;
            changed = true;

            Serial.print("💤 Sensor a dormido: ");
            Serial.println(node.name);
        }
    }

    if (changed)
    {
        Serial.print("   Activos: ");
        Serial.print(sysState.activeNodes);
        Serial.print(", Dormidos: ");
        Serial.println(sysState.dormantNodes);
    }
}

// ============================================
// ОБНОВЛЕНИЕ ВРЕМЕНИ ПОСЛЕДНЕЙ ОТПРАВКИ
// ============================================
void ED_DataManager::updateLastTransmit()
{
    sysState.lastTransmit = "hace 0s";
}