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
const char *ssid = "Mechanic";        // Имя сети нашего ESP32-S3
const char *password = "12345678";    // Пароль сети ESP32-S3
const char *serverIP = "192.168.4.1"; // IP-адрес ESP32-S3 в режиме AP

// Уникальное имя этого датчика
const char *sensorName = "dht11_nodemcu_garage";

unsigned long lastSendTime = 0;
const unsigned long sendInterval = 10000; // Отправляем данные каждые 10 секунд

// ЗАЧЕМ: Создаем экземпляр WiFiClient.
// Новое ядро ESP8266 требует его явной передачи в HTTPClient для управления соединением.
WiFiClient wifiClient;

// ============================================
// ПРОТОТИПЫ ФУНКЦИЙ
// ============================================
void registrarSensor();
void enviarDatos(float temp, float hum);

// ============================================
// SETUP
// ============================================
void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=================================");
  Serial.println("📡 Узел датчика: ESP8266 + DHT11");
  Serial.println("=================================");

  dht.begin();
  Serial.println("✅ DHT11 инициализирован");

  Serial.print("🔄 Подключение к сети '");
  Serial.print(ssid);
  Serial.println("'...");

  WiFi.begin(ssid, password);

  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 20)
  {
    delay(500);
    Serial.print(".");
    timeout++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\n✅ Подключено к Wi-Fi!");
    Serial.print("🌐 Мой IP-адрес: ");
    Serial.println(WiFi.localIP());

    registrarSensor();
  }
  else
  {
    Serial.println("\n❌ Ошибка: Не удалось подключиться к Wi-Fi. Перезагрузка...");
    ESP.restart();
  }
}

// ============================================
// LOOP
// ============================================
void loop()
{
  if (millis() - lastSendTime >= sendInterval)
  {
    lastSendTime = millis();

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t))
    {
      Serial.println("❌ Ошибка чтения данных с DHT11!");
      return;
    }

    enviarDatos(t, h);
  }

  delay(100);
}

// ============================================
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
// ============================================

void registrarSensor()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    String url = "http://" + String(serverIP) + "/register";

    // ЗАЧЕМ: Передаем wifiClient первым аргументом, как того требует новое ядро ESP8266 3.x
    http.begin(wifiClient, url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "nombre=" + String(sensorName);

    Serial.print("📤 Регистрация датчика на сервере... ");
    int httpCode = http.POST(postData);

    if (httpCode > 0)
    {
      String response = http.getString();
      Serial.println("✅ Успешно! Ответ сервера: " + response);
    }
    else
    {
      Serial.println("❌ Ошибка регистрации. Код: " + String(httpCode));
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

    // ЗАЧЕМ: И здесь передаем wifiClient первым аргументом
    http.begin(wifiClient, url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "nombre=" + String(sensorName) +
                      "&temperatura=" + String(temp, 1) +
                      "&humedad=" + String(hum, 1);

    Serial.print("📤 Отправка данных (T: ");
    Serial.print(temp, 1);
    Serial.print("°C, H: ");
    Serial.print(hum, 1);
    Serial.println("%)... ");

    int httpCode = http.POST(postData);

    if (httpCode > 0)
    {
      String response = http.getString();
      Serial.println("✅ Успешно! Ответ: " + response);
    }
    else
    {
      Serial.println("❌ Ошибка отправки. Код: " + String(httpCode));

      if (httpCode == -1)
      {
        Serial.println("🔄 Попытка переподключения к Wi-Fi...");
        WiFi.disconnect();
        WiFi.begin(ssid, password);
      }
    }
    http.end();
  }
}