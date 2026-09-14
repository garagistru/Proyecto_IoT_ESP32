/*
 * ============================================================
 *  ESP8266 + DHT11 — ДАТЧИК ТЕМПЕРАТУРЫ/ВЛАЖНОСТИ
 *  УЗЕЛ: microclima_1 (измени через CONFIG_DEVICE_NAME)
 *  ПЛАТА: NodeMCU v2 (ESP8266)
 * ============================================================
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>

// ============================================================
// НАСТРОЙКИ
// ============================================================
#define CONFIG_DEVICE_NAME "microclima_1"

#define DHT_PIN 2 // D4 (GPIO2)
#define DHT_TYPE DHT11
#define BOOT_BUTTON 5 // D1 (GPIO5)

const char *WIFI_SSID = "Mechanic";
const char *WIFI_PASS = "12345678";
const char *GATEWAY_URL = "http://192.168.4.1";

const unsigned long INTERVAL_SEND_MS = 3600000UL;
const unsigned long DEBOUNCE_MS = 50;
const unsigned long WIFI_RETRY_DELAY_MS = 5000;

// ============================================================
// ГЛОБАЛЬНЫЕ ОБЪЕКТЫ
// ============================================================
DHT dht(DHT_PIN, DHT_TYPE);
WiFiClient wifiClient;

String deviceMac;
String bootId;

bool lastButtonState = HIGH;
unsigned long lastDebounceMs = 0;

unsigned long lastSendMs = 0;

// ============================================================
// ВСПОМОГАТЕЛЬНЫЕ
// ============================================================
String getMacAddress()
{
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  mac.toUpperCase();
  return mac;
}

String generateBootId()
{
  uint32_t rnd = RANDOM_REG32;
  char buf[9];
  snprintf(buf, sizeof(buf), "%08X", rnd);
  return String(buf);
}

// ============================================================
// WI-FI — переименовано, чтобы не конфликтовать с библиотекой
// ============================================================
enum WifiConnState
{
  WIFI_STATE_IDLE,
  WIFI_STATE_CONNECTING,
  WIFI_STATE_CONNECTED
};
WifiConnState wifiState = WIFI_STATE_IDLE;
unsigned long wifiTimer = 0;

void connectWiFiAsync()
{
  switch (wifiState)
  {
  case WIFI_STATE_CONNECTED:
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("⚠️ WiFi: потерян");
      wifiState = WIFI_STATE_IDLE;
      wifiTimer = millis();
    }
    break;

  case WIFI_STATE_IDLE:
    if (millis() - wifiTimer < WIFI_RETRY_DELAY_MS)
      return;
    Serial.print("📡 WiFi: подключение...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    wifiState = WIFI_STATE_CONNECTING;
    wifiTimer = millis();
    break;

  case WIFI_STATE_CONNECTING:
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println(" ✅");
      Serial.printf("   IP: %s\n", WiFi.localIP().toString().c_str());
      wifiState = WIFI_STATE_CONNECTED;
      lastSendMs = 0;
    }
    else if (millis() - wifiTimer > 20000)
    {
      Serial.println(" ❌ таймаут");
      wifiState = WIFI_STATE_IDLE;
      wifiTimer = millis();
    }
    break;
  }
}

// ============================================================
// ОТПРАВКА ДАННЫХ
// ============================================================
bool sendData(float temp, float hum)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("⚠️ Нет Wi-Fi, пропуск");
    return false;
  }

  HTTPClient http;
  String url = String(GATEWAY_URL) + "/data";
  http.begin(wifiClient, url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.setTimeout(3000);

  String postData = "name=" + String(CONFIG_DEVICE_NAME) +
                    "&mac=" + deviceMac +
                    "&temp=" + String(temp, 1) +
                    "&hum=" + String(hum, 1) +
                    "&sensor_uptime_s=" + String(millis() / 1000) +
                    "&boot_id=" + bootId;

  Serial.print("📤 Отправка (T=");
  Serial.print(temp, 1);
  Serial.print("°C, H=");
  Serial.print(hum, 1);
  Serial.print("%)... ");

  int code = http.POST(postData);
  String response = http.getString();
  http.end();

  if (code == 200)
  {
    Serial.println("✅ OK");
    return true;
  }
  else
  {
    Serial.printf("❌ Код %d\n", code);
    return false;
  }
}

void sendCurrentReading()
{
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();

  if (isnan(hum) || isnan(temp))
  {
    Serial.println("❌ Ошибка чтения DHT11");
    return;
  }

  sendData(temp, hum);
}

// ============================================================
// КНОПКА
// ============================================================
void handleButton()
{
#if BOOT_BUTTON >= 0
  bool currentState = digitalRead(BOOT_BUTTON);
  if (currentState != lastButtonState)
  {
    lastDebounceMs = millis();
  }
  if ((millis() - lastDebounceMs) > DEBOUNCE_MS)
  {
    static bool stableState = HIGH;
    if (currentState != stableState)
    {
      stableState = currentState;
      if (stableState == LOW)
      {
        Serial.println("🔘 Кнопка: отправка по требованию");
        sendCurrentReading();
      }
    }
  }
  lastButtonState = currentState;
#endif
}

// ============================================================
// SETUP
// ============================================================
void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=================================");
  Serial.println("📡 ESP8266 + DHT11 — ДАТЧИК");
  Serial.printf("   Узел: %s\n", CONFIG_DEVICE_NAME);
  Serial.println("=================================");

  dht.begin();
  Serial.println("✅ DHT11 инициализирован");

#if BOOT_BUTTON >= 0
  pinMode(BOOT_BUTTON, INPUT_PULLUP);
  Serial.printf("✅ Кнопка на GPIO%d\n", BOOT_BUTTON);
#endif

  deviceMac = getMacAddress();
  bootId = generateBootId();
  Serial.printf("🆔 MAC: %s\n", deviceMac.c_str());
  Serial.printf("🔑 Boot ID: %s\n", bootId.c_str());

  connectWiFiAsync();
  unsigned long t = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t < 15000)
  {
    delay(200);
    Serial.print(".");
  }
  Serial.println(WiFi.status() == WL_CONNECTED ? " ✅" : " ❌");

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("📤 Первая отправка после старта");
    sendCurrentReading();
  }

  lastSendMs = millis();
  Serial.println("✅ Готов");
  Serial.println("================================");
}

// ============================================================
// LOOP
// ============================================================
void loop()
{
  handleButton();
  connectWiFiAsync();

  if (millis() - lastSendMs >= INTERVAL_SEND_MS)
  {
    lastSendMs = millis();
    sendCurrentReading();
  }

  static unsigned long lastHeapLog = 0;
  if (millis() - lastHeapLog > 600000UL)
  {
    lastHeapLog = millis();
    Serial.printf("💾 Free heap: %u bytes\n", ESP.getFreeHeap());
  }

  delay(10);
}