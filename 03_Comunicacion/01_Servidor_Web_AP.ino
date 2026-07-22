#include <WiFi.h>
#include <WebServer.h>

// ЗАЧЕМ: Настройки точки доступа (ESP32 станет WiFi-роутером)
const char* ssid_ap = "Mechanic";
const char* password_ap = "12345678";

// ЗАЧЕМ: Создаём веб-сервер на стандартном HTTP порту 80
WebServer server(80);

// ЗАЧЕМ: Пин встроенного светодиода для индикации получения данных
#define LED_BUILTIN 48

// ЗАЧЕМ: Счётчик полученных запросов (для демонстрации)
int requestCount = 0;

void setup() {
  // ЗАЧЕМ: Инициализируем Serial для отладки (через UART, так как USB CDC = Disabled)
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n\n=================================");
  Serial.println("ESP32-S3 Web Server (AP Mode)");
  Serial.println("=================================");
  
  // ЗАЧЕМ: Настраиваем пин светодиода как выход
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); // Выключаем светодиод
  
  // ЗАЧЕМ: Создаём точку доступа (Access Point)
  // ESP32 будет раздавать WiFi сеть с именем ssid_ap
  WiFi.softAP(ssid_ap, password_ap);
  
  // ЗАЧЕМ: Получаем IP-адрес, который назначил себе ESP32
  // Обычно это 192.168.4.1 для режима точки доступа
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("✅ Точка доступа создана!\n");
  Serial.print("📶 Имя сети (SSID): ");
  Serial.println(ssid_ap);
  Serial.print("🔑 Пароль: ");
  Serial.println(password_ap);
  Serial.print("🌐 IP-адрес сервера: http://");
  Serial.println(myIP);
  Serial.println("=================================\n");
  
  // ЗАЧЕМ: Обработчик GET-запроса на корневой путь "/"
  // Когда ты откроешь http://192.168.4.1 в браузере, выполнится этот код
  server.on("/", HTTP_GET, []() {
    Serial.println("📥 Получен GET-запрос на /");
    
    // ЗАЧЕМ: Формируем HTML-страницу с формой для отправки данных
    String html = "<!DOCTYPE html><html><head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>ESP32 Web Server</title>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; max-width: 600px; margin: 50px auto; padding: 20px; }";
    html += "h1 { color: #333; }";
    html += "input, button { padding: 10px; margin: 5px 0; width: 100%; box-sizing: border-box; }";
    html += "button { background-color: #4CAF50; color: white; border: none; cursor: pointer; font-size: 16px; }";
    html += "button:hover { background-color: #45a049; }";
    html += ".info { background-color: #f0f0f0; padding: 15px; border-radius: 5px; margin: 20px 0; }";
    html += "</style></head><body>";
    html += "<h1>🚀 ESP32-S3 Web Server</h1>";
    html += "<div class='info'>";
    html += "<p><strong>Статус:</strong> Сервер работает!</p>";
    html += "<p><strong>Получено запросов:</strong> " + String(requestCount) + "</p>";
    html += "</div>";
    html += "<h2>Отправить данные на сервер:</h2>";
    html += "<form action='/data' method='POST'>";
    html += "<label>Текст сообщения:</label>";
    html += "<input type='text' name='message' placeholder='Введите текст...' required>";
    html += "<button type='submit'>Отправить POST-запрос</button>";
    html += "</form>";
    html += "</body></html>";
    
    // ЗАЧЕМ: Отправляем HTML-страницу клиенту (браузеру)
    server.send(200, "text/html", html);
  });
  
  // ЗАЧЕМ: Обработчик POST-запроса на путь "/data"
  // Когда форма отправит данные, выполнится этот код
  server.on("/data", HTTP_POST, []() {
    requestCount++;
    Serial.println("\n📥 Получен POST-запрос на /data");
    
    // ЗАЧЕМ: Извлекаем данные из тела запроса
    // Если в форме есть поле "message", получаем его значение
    if (server.hasArg("message")) {
      String message = server.arg("message");
      Serial.print("💬 Сообщение: ");
      Serial.println(message);
      
      // ЗАЧЕМ: Мигаем светодиодом, чтобы показать, что данные получены
      digitalWrite(LED_BUILTIN, HIGH);
      delay(200);
      digitalWrite(LED_BUILTIN, LOW);
    } else {
      // ЗАЧЕМ: Если данные пришли в "сыром" виде (не из формы)
      String body = server.arg("plain");
      Serial.print("📦 Сырые данные: ");
      Serial.println(body);
    }
    
    // ЗАЧЕМ: Отправляем ответ клиенту
    String response = "<!DOCTYPE html><html><body>";
    response += "<h2>✅ Данные получены!</h2>";
    response += "<p>Количество запросов: " + String(requestCount) + "</p>";
    response += "<a href='/'>← Вернуться на главную</a>";
    response += "</body></html>";
    
    server.send(200, "text/html", response);
    Serial.println("✅ Ответ отправлен клиенту\n");
  });
  
  // ЗАЧЕМ: Обработчик для несуществующих страниц (404)
  server.onNotFound([]() {
    Serial.print("❌ Запрошена несуществующая страница: ");
    Serial.println(server.uri());
    server.send(404, "text/plain", "404: Страница не найдена");
  });
  
  // ЗАЧЕМ: Запускаем сервер (начинаем слушать порт 80)
  server.begin();
  Serial.println("🚀 HTTP сервер запущен на порту 80");
  Serial.println("👀 Ожидание подключений...\n");
}

void loop() {
  // ЗАЧЕМ: Обрабатываем входящие запросы от клиентов
  // Эта функция должна вызываться постоянно в loop()
  server.handleClient();
  
  // ЗАЧЕМ: Небольшая задержка, чтобы не перегружать процессор
  delay(10);
}