#include "ED_WebServer.h"
#include "ED_ServerLink.h"

extern ED_DataManager dataManager;
extern ED_Display display;
extern unsigned long lastReceiveTime;

ED_WebServer::ED_WebServer() : server(80) {}

void ED_WebServer::begin(const char *ssid, const char *password)
{
    WiFi.softAP(ssid, password);
    delay(100);

    server.on("/time", HTTP_GET, handleTime);
    server.on("/data", HTTP_POST, handleData);
    server.on("/status", HTTP_POST, handleStatus);
    server.on("/devices", HTTP_GET, handleDevices);
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    server.begin();
    Serial.println("✅ WebServer: " + WiFi.softAPIP().toString());
}

void ED_WebServer::update() {}

String ED_WebServer::getTimeJson()
{
    unsigned long s = millis() / 1000;
    int h = (s / 3600) % 24;
    int m = (s % 3600) / 60;
    int sec = s % 60;
    char buf[80];
    snprintf(buf, sizeof(buf),
             "{\"hour\":%d,\"minute\":%d,\"second\":%d,\"full\":\"%02d:%02d:%02d\"}",
             h, m, sec, h, m, sec);
    return String(buf);
}

void ED_WebServer::handleTime(AsyncWebServerRequest *request)
{
    request->send(200, "application/json", getTimeJson());
}

// ⬅️ ТРАНЗИТ: собираем ВСЕ параметры в rawBody
void ED_WebServer::handleData(AsyncWebServerRequest *request)
{
    String name = "";
    String mac = "";
    String rawBody = "";

    // Проходим по ВСЕМ параметрам POST
    int params = request->params();
    bool first = true;
    for (int i = 0; i < params; i++)
    {
        const AsyncWebParameter *p = request->getParam(i);
        if (!p->isPost())
            continue; // Только POST-тело

        if (p->name() == "name")
            name = p->value();
        if (p->name() == "mac")
            mac = p->value();

        if (!first)
            rawBody += "&";
        rawBody += p->name() + "=" + p->value();
        first = false;
    }

    if (name.length() == 0)
    {
        request->send(400, "application/json", "{\"error\":\"no name\"}");
        return;
    }

    Serial.printf("\n📊 RAW от %s (MAC: %s)\n", name.c_str(), mac.c_str());

    dataManager.registerNode(name, mac);
    dataManager.queueRawPacket(rawBody, name, mac);
    lastReceiveTime = millis();

    request->send(200, "application/json", "{\"status\":\"ok\"}");
    display.drawRealTimeData();
}

void ED_WebServer::handleStatus(AsyncWebServerRequest *request)
{
    String nombre = request->arg("nombre");
    String status = request->arg("status");

    if (nombre.length() == 0 || status.length() == 0)
    {
        request->send(400, "application/json", "{\"error\":\"Missing fields\"}");
        return;
    }

    Serial.printf("📡 %s → %s\n", nombre.c_str(), status.c_str());
    dataManager.setNodeStatus(nombre, status);
    lastReceiveTime = millis();

    request->send(200, "application/json", "{\"status\":\"ok\"}");
    display.drawRealTimeData();
}

void ED_WebServer::handleDevices(AsyncWebServerRequest *request)
{
    request->send(200, "application/json", dataManager.getDevicesJson());
}