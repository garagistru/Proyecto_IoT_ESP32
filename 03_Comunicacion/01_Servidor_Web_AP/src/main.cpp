#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h> // Для работы с JSON (автоматически скачается PlatformIO)

// ============================================
// НАСТРОЙКИ СИСТЕМЫ
// ============================================
const char *ssid_ap = "Mechanic";
const char *password_ap = "12345678";

#define PIN_LED 48
#define MAX_SENSORES 10      // Максимальное количество датчиков в сети
#define MAX_LECTURAS 3       // Сколько последних показаний хранить для каждого датчика
#define TIMEOUT_SENSOR 30000 // Таймаут для статуса "apagado" (30 секунд)

// ============================================
// СТРУКТУРЫ ДАННЫХ
// ============================================

// Структура для одного показания датчика
struct Lectura
{
  float temperatura;
  float humedad;
  unsigned long timestamp; // Время получения (millis)
};

// Структура для одного датчика
struct Sensor
{
  String nombre;                  // Имя датчика (например, "dht11_garage")
  Lectura lecturas[MAX_LECTURAS]; // Последние 3 показания
  int cantidadLecturas;           // Сколько показаний сейчас хранится (0-3)
  unsigned long ultimaActividad;  // Время последней активности
  bool activo;                    // Статус: true = activo, false = apagado
};

// Массив всех датчиков в сети
Sensor sensores[MAX_SENSORES];
int cantidadSensores = 0; // Сколько датчиков зарегистрировано

// ============================================
// ВЕБ-СЕРВЕР
// ============================================
WebServer server(80);
int requestCount = 0;

// ============================================
// ФУНКЦИИ ДЛЯ РАБОТЫ С ДАТЧИКАМИ
// ============================================

// Найти датчик по имени (возвращает индекс или -1 если не найден)
int buscarSensor(String nombre)
{
  for (int i = 0; i < cantidadSensores; i++)
  {
    if (sensores[i].nombre == nombre)
    {
      return i;
    }
  }
  return -1; // Не найден
}

// Зарегистрировать новый датчик (если еще не зарегистрирован)
int registrarSensor(String nombre)
{
  int indice = buscarSensor(nombre);

  if (indice >= 0)
  {
    // Датчик уже зарегистрирован, просто обновляем время активности
    sensores[indice].ultimaActividad = millis();
    sensores[indice].activo = true;
    return indice;
  }

  // Новый датчик, добавляем в массив
  if (cantidadSensores >= MAX_SENSORES)
  {
    Serial.println("❌ Превышен максимум датчиков!");
    return -1;
  }

  int nuevoIndice = cantidadSensores;
  sensores[nuevoIndice].nombre = nombre;
  sensores[nuevoIndice].cantidadLecturas = 0;
  sensores[nuevoIndice].ultimaActividad = millis();
  sensores[nuevoIndice].activo = true;
  cantidadSensores++;

  Serial.print("✅ Зарегистрирован новый датчик: ");
  Serial.println(nombre);

  return nuevoIndice;
}

// Добавить показание для датчика
void agregarLectura(int indiceSensor, float temp, float hum)
{
  Sensor &sensor = sensores[indiceSensor];

  // Сдвигаем старые показания влево (если уже 3, удаляем самое старое)
  if (sensor.cantidadLecturas < MAX_LECTURAS)
  {
    sensor.cantidadLecturas++;
  }

  // Сдвигаем массив
  for (int i = sensor.cantidadLecturas - 1; i > 0; i--)
  {
    sensor.lecturas[i] = sensor.lecturas[i - 1];
  }

  // Добавляем новое показание в начало
  sensor.lecturas[0].temperatura = temp;
  sensor.lecturas[0].humedad = hum;
  sensor.lecturas[0].timestamp = millis();

  // Обновляем статус
  sensor.ultimaActividad = millis();
  sensor.activo = true;

  Serial.print("📊 Показание от ");
  Serial.print(sensor.nombre);
  Serial.print(": Temp=");
  Serial.print(temp);
  Serial.print("°C, Hum=");
  Serial.print(hum);
  Serial.println("%");
}

// Проверить статус всех датчиков (вызывается в loop)
void verificarEstados()
{
  unsigned long ahora = millis();

  for (int i = 0; i < cantidadSensores; i++)
  {
    if (sensores[i].activo)
    {
      if (ahora - sensores[i].ultimaActividad > TIMEOUT_SENSOR)
      {
        sensores[i].activo = false;
        Serial.print("⚠️ Датчик ");
        Serial.print(sensores[i].nombre);
        Serial.println(" перешел в статус: apagado");
      }
    }
  }
}

// ============================================
// HTML-ИНТЕРФЕЙС (Генерация страницы)
// ============================================

String generarPaginaPrincipal()
{
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Panel de Sensores - ESP32-S3</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; max-width: 900px; margin: 20px auto; padding: 20px; background-color: #f5f5f5; }";
  html += "h1 { color: #333; text-align: center; }";
  html += ".sensor-card { background: white; border-radius: 8px; padding: 15px; margin: 10px 0; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }";
  html += ".sensor-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 10px; }";
  html += ".sensor-nombre { font-size: 18px; font-weight: bold; color: #2196F3; }";
  html += ".estado-activo { color: #4CAF50; font-weight: bold; }";
  html += ".estado-apagado { color: #f44336; font-weight: bold; }";
  html += ".lectura { background: #f9f9f9; padding: 8px; margin: 5px 0; border-radius: 4px; font-size: 14px; }";
  html += ".timestamp { color: #666; font-size: 12px; }";
  html += ".info-panel { background: #e3f2fd; padding: 15px; border-radius: 8px; margin-bottom: 20px; }";
  html += "</style></head><body>";

  html += "<h1> Panel de Sensores IoT</h1>";

  // Информационная панель
  html += "<div class='info-panel'>";
  html += "<p><strong>Всего датчиков:</strong> " + String(cantidadSensores) + " / " + String(MAX_SENSORES) + "</p>";
  html += "<p><strong>Активных:</strong> ";
  int activos = 0;
  for (int i = 0; i < cantidadSensores; i++)
  {
    if (sensores[i].activo)
      activos++;
  }
  html += String(activos) + "</p>";
  html += "<p><strong>Всего запросов:</strong> " + String(requestCount) + "</p>";
  html += "</div>";

  // Список датчиков
  if (cantidadSensores == 0)
  {
    html += "<div class='sensor-card'><p>Нет зарегистрированных датчиков. Ожидание подключения...</p></div>";
  }
  else
  {
    for (int i = 0; i < cantidadSensores; i++)
    {
      html += "<div class='sensor-card'>";
      html += "<div class='sensor-header'>";
      html += "<span class='sensor-nombre'>📡 " + sensores[i].nombre + "</span>";

      if (sensores[i].activo)
      {
        html += "<span class='estado-activo'>● ACTIVO</span>";
      }
      else
      {
        html += "<span class='estado-apagado'>● APAGADO</span>";
      }

      html += "</div>";

      // Показываем последние показания
      if (sensores[i].cantidadLecturas > 0)
      {
        html += "<h3>Последние показания:</h3>";
        for (int j = 0; j < sensores[i].cantidadLecturas; j++)
        {
          unsigned long tiempoTranscurrido = millis() - sensores[i].lecturas[j].timestamp;
          int segundos = tiempoTranscurrido / 1000;

          html += "<div class='lectura'>";
          html += "🌡️ Temp: " + String(sensores[i].lecturas[j].temperatura, 1) + "°C";
          html += " | 💧 Hum: " + String(sensores[i].lecturas[j].humedad, 1) + "%";
          html += "<br><span class='timestamp'>⏱️ " + String(segundos) + " сек. назад</span>";
          html += "</div>";
        }
      }
      else
      {
        html += "<p>Нет данных</p>";
      }

      html += "</div>";
    }
  }

  html += "</body></html>";
  return html;
}

// ============================================
// SETUP
// ============================================

void setup()
{
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n\n=================================");
  Serial.println("ESP32-S3 Gateway - Sistema IoT");
  Serial.println("=================================");

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  // Создаем точку доступа
  WiFi.softAP(ssid_ap, password_ap);
  IPAddress myIP = WiFi.softAPIP();

  Serial.print("✅ Точка доступа создана!\n");
  Serial.print("📶 SSID: ");
  Serial.println(ssid_ap);
  Serial.print("🔑 Password: ");
  Serial.println(password_ap);
  Serial.print(" IP: http://");
  Serial.println(myIP);
  Serial.println("=================================\n");

  // ============================================
  // ЭНДПОИНТЫ API
  // ============================================

  // Главная страница (панель мониторинга)
  server.on("/", HTTP_GET, []()
            {
    Serial.println("📥 GET / - Запрос главной страницы");
    server.send(200, "text/html", generarPaginaPrincipal()); });

  // Регистрация датчика (POST /register)
  server.on("/register", HTTP_POST, []()
            {
    requestCount++;
    Serial.println("\n📥 POST /register - Регистрация датчика");
    
    if (server.hasArg("nombre")) {
      String nombreSensor = server.arg("nombre");
      int indice = registrarSensor(nombreSensor);
      
      if (indice >= 0) {
        Serial.print("✅ Датчик зарегистрирован: ");
        Serial.println(nombreSensor);
        server.send(200, "text/plain", "OK: Sensor registrado");
      } else {
        server.send(500, "text/plain", "ERROR: Maximo de sensores alcanzado");
      }
    } else {
      server.send(400, "text/plain", "ERROR: Falta parametro 'nombre'");
    } });

  // Прием данных от датчика (POST /data)
  server.on("/data", HTTP_POST, []()
            {
    requestCount++;
    Serial.println("\n📥 POST /data - Получение данных");
    
    // Проверяем, есть ли имя датчика
    if (!server.hasArg("nombre")) {
      server.send(400, "text/plain", "ERROR: Falta parametro 'nombre'");
      return;
    }
    
    String nombreSensor = server.arg("nombre");
    
    // Регистрируем или находим датчик
    int indice = registrarSensor(nombreSensor);
    if (indice < 0) {
      server.send(500, "text/plain", "ERROR: No se pudo registrar sensor");
      return;
    }
    
    // Проверяем, есть ли данные
    if (server.hasArg("temperatura") && server.hasArg("humedad")) {
      float temp = server.arg("temperatura").toFloat();
      float hum = server.arg("humedad").toFloat();
      
      // Добавляем показание
      agregarLectura(indice, temp, hum);
      
      // Мигаем светодиодом
      digitalWrite(PIN_LED, HIGH);
      delay(100);
      digitalWrite(PIN_LED, LOW);
      
      server.send(200, "text/plain", "OK: Datos recibidos");
    } else {
      server.send(400, "text/plain", "ERROR: Faltan parametros 'temperatura' o 'humedad'");
    } });

  // API для получения данных в формате JSON (для будущего Ubuntu сервера)
  server.on("/api/sensores", HTTP_GET, []()
            {
    Serial.println("📥 GET /api/sensores - Запрос JSON API");
    
    // Создаем JSON ответ
    String json = "[";
    for (int i = 0; i < cantidadSensores; i++) {
      if (i > 0) json += ",";
      json += "{";
      json += "\"nombre\":\"" + sensores[i].nombre + "\",";
      json += "\"activo\":" + String(sensores[i].activo ? "true" : "false") + ",";
      json += "\"lecturas\":[";
      for (int j = 0; j < sensores[i].cantidadLecturas; j++) {
        if (j > 0) json += ",";
        json += "{";
        json += "\"temp\":" + String(sensores[i].lecturas[j].temperatura, 1) + ",";
        json += "\"hum\":" + String(sensores[i].lecturas[j].humedad, 1) + ",";
        json += "\"ts\":" + String(sensores[i].lecturas[j].timestamp);
        json += "}";
      }
      json += "]}";
    }
    json += "]";
    
    server.send(200, "application/json", json); });

  // Обработчик 404
  server.onNotFound([]()
                    {
    Serial.print(" 404: ");
    Serial.println(server.uri());
    server.send(404, "text/plain", "404: Not Found"); });

  // Запускаем сервер
  server.begin();
  Serial.println(" HTTP сервер запущен на порту 80");
  Serial.println("👀 Ожидание подключения датчиков...\n");
}

// ============================================
// LOOP
// ============================================

void loop()
{
  server.handleClient();
  verificarEstados(); // Проверяем статусы датчиков
  delay(10);
}