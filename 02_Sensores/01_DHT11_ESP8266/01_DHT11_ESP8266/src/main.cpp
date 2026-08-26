#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>

// ============================================
// НАСТРОЙКИ ДАТЧИКА
// ============================================
#define DHTPIN 2      // D4 на NodeMCU (GPIO2)
#define DHTTYPE DHT11 // Тип датчика
DHT dht(DHTPIN, DHTTYPE);

// ============================================
// НАСТРОЙКИ СЕТИ
// ============================================
const char *ssid = "Mechanic";
const char *password = "12345678";
const char *serverIP = "192.168.4.1";

// Уникальное имя этого датчика
const char *sensorName = "microclima_1";

unsigned long lastSendTime = 0;
const unsigned long sendInterval = 3600000; // 1 час (60 * 60 * 1000)

WiFiClient wifiClient;

void registrarSensor();
void enviarDatos(float temp, float hum);

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=================================");
  Serial.println("📡 Sensor Node: ESP8266 + DHT11");
  Serial.println("=================================");

  dht.begin();
  Serial.println("✅ DHT11 iniciado");

  Serial.print("🔄 Conectando a '");
  Serial.print(ssid);
  Serial.println("'...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 30)
  {
    delay(500);
    Serial.print(".");
    timeout++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\n✅ Conectado a Wi-Fi!");
    Serial.print("🌐 IP: ");
    Serial.println(WiFi.localIP());
    registrarSensor();
  }
  else
  {
    Serial.println("\n❌ Error de conexión. Reiniciando...");
    ESP.restart();
  }
}

void loop()
{
  if (millis() - lastSendTime >= sendInterval)
  {
    lastSendTime = millis();

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t))
    {
      Serial.println("❌ Error al leer DHT11!");
      return;
    }

    enviarDatos(t, h);
  }

  delay(100);
}

void registrarSensor()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    String url = "http://" + String(serverIP) + "/register";
    http.begin(wifiClient, url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "nombre=" + String(sensorName);

    Serial.print("📤 Registrando sensor... ");
    int httpCode = http.POST(postData);

    if (httpCode > 0)
    {
      String response = http.getString();
      Serial.println("✅ Éxito! Respuesta: " + response);
    }
    else
    {
      Serial.println("❌ Error de registro. Código: " + String(httpCode));
    }
    http.end();
  }
}

void enviarDatos(float temp, float hum)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    String url = "http://" + String(serverIP) + "/data";
    http.begin(wifiClient, url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "nombre=" + String(sensorName) +
                      "&temperatura=" + String(temp, 1) +
                      "&humedad=" + String(hum, 1);

    Serial.print("📤 Enviando (T: ");
    Serial.print(temp, 1);
    Serial.print("°C, H: ");
    Serial.print(hum, 1);
    Serial.println("%)... ");

    int httpCode = http.POST(postData);

    if (httpCode > 0)
    {
      String response = http.getString();
      Serial.println("✅ Éxito! Respuesta: " + response);
    }
    else
    {
      Serial.println("❌ Error de envío. Código: " + String(httpCode));
    }
    http.end();
  }
}