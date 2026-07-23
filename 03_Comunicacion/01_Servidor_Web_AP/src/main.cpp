#include <Arduino.h> // ОБЯЗАТЕЛЬНО для PlatformIO!
#include <WiFi.h>
#include <WebServer.h>

const char *ssid_ap = "Mechanic";
const char *password_ap = "12345678";

WebServer server(80);
// #define LED_BUILTIN 48
#define PIN_LED 48 //   замена
int requestCount = 0;

void setup()
{
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n\n=================================");
  Serial.println("ESP32-S3 Web Server (AP Mode)");
  Serial.println("=================================");

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  WiFi.softAP(ssid_ap, password_ap);
  IPAddress myIP = WiFi.softAPIP();

  Serial.print("✅ Точка доступа создана!\n");
  Serial.print("📶 Имя сети (SSID): ");
  Serial.println(ssid_ap);
  Serial.print("🔑 Пароль: ");
  Serial.println(password_ap);
  Serial.print("🌐 IP-адрес сервера: http://");
  Serial.println(myIP);
  Serial.println("=================================\n");

  server.on("/", HTTP_GET, []()
            {
    Serial.println("📥 Получен GET-запрос на /");
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>ESP32 Web Server</title>";
    html += "<style>body { font-family: Arial; max-width: 600px; margin: 50px auto; padding: 20px; }";
    html += "input, button { padding: 10px; margin: 5px 0; width: 100%; box-sizing: border-box; }";
    html += "button { background-color: #4CAF50; color: white; border: none; cursor: pointer; }</style></head><body>";
    html += "<h1>🚀 ESP32-S3 Web Server</h1>";
    html += "<p><strong>Получено запросов:</strong> " + String(requestCount) + "</p>";
    html += "<form action='/data' method='POST'>";
    html += "<label>Текст сообщения:</label>";
    html += "<input type='text' name='message' placeholder='Введите текст...' required>";
    html += "<button type='submit'>Отправить POST-запрос</button>";
    html += "</form></body></html>";
    server.send(200, "text/html", html); });

  server.on("/data", HTTP_POST, []()
            {
    requestCount++;
    Serial.println("\n📥 Получен POST-запрос на /data");
    
    if (server.hasArg("message")) {
      String message = server.arg("message");
      Serial.print("💬 Сообщение: ");
      Serial.println(message);
      digitalWrite(PIN_LED, HIGH);
      delay(200);
      digitalWrite(PIN_LED, LOW);
    } else {
      String body = server.arg("plain");
      Serial.print("📦 Сырые данные: ");
      Serial.println(body);
    }
    
    String response = "<!DOCTYPE html><html><body><h2>✅ Данные получены!</h2>";
    response += "<p>Количество запросов: " + String(requestCount) + "</p>";
    response += "<a href='/'>← Вернуться на главную</a></body></html>";
    server.send(200, "text/html", response);
    Serial.println("✅ Ответ отправлен клиенту\n"); });

  server.onNotFound([]()
                    {
    Serial.print("❌ Запрошена несуществующая страница: ");
    Serial.println(server.uri());
    server.send(404, "text/plain", "404: Not Found"); });

  server.begin();
  Serial.println("🚀 HTTP сервер запущен на порту 80");
  Serial.println("👀 Ожидание подключений...\n");
}

void loop()
{
  server.handleClient();
  delay(10);
}