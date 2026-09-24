/*
ESP32-C6 ДАТЧИК СТАНКА — УНИВЕРСАЛЬНЫЙ УЗЕЛ
ПЛАТА: ESP32-C6 SuperMini
ОСОБЕННОСТИ:
- Кольцевой буфер очереди (без массовой записи NVS)
- Отказ от String в hot-path (нет фрагментации кучи)
- Неблокирующий антидребезг кнопок
- WiFi.setSleep(NONE) — стабильность 24/7
- Минимизированный дисплей (без перекрытий)
- Аппаратная кнопка перезагрузки (GPIO9 — безопасный пин)
- АВТО-РЕИНИЦИАЛИЗАЦИЯ OLED после HW reset (разблокировка I2C)
Команды: setname, info, send, clear, resetshift, sync, reboot
============================================================
*/

// ============================================================
// ВЕРСИЯ ПРОШИВКИ (ИЗМЕНЯТЬ ЗДЕСЬ)
// ============================================================
#define FIRMWARE_VERSION "v3.6"

// ============================================================
// КОМАНДЫ УПРАВЛЕНИЯ (Serial Monitor, 115200)
// ============================================================
// Ввод: одна строка, Enter в конце. Регистр имеет значение.
// (Список команд см. в функции handleSerialCommands)
// ============================================================

#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <esp_system.h>

// ============================================================
// ПИНЫ ПЛАТЫ (НЕ МЕНЯТЬ)
// ============================================================
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define OLED_RESET      -1
#define OLED_ADDR       0x3C
#define OLED_SDA        4
#define OLED_SCL        5
#define BUTTON_COUNT    14
#define BUTTON_SHIFT    15
#define BUTTON_REBOOT   9   // v3.5+: Перезагрузка (NO, INPUT_PULLUP)
                            // GPIO9 — безопасный пин, НЕ strapping,
                            // НЕ используется UART/USB/JTAG/SPI.
                            // ⚠️ НЕ использовать GPIO8 (strapping boot pin)!
                            // ⚠️ GPIO10 физически отсутствует на SuperMini!

// ============================================================
// WI-FI
// ============================================================
const char* WIFI_SSID   = "Mechanic";
const char* WIFI_PASS   = "12345678";
const char* GATEWAY_URL = "http://192.168.4.1";

// ============================================================
// ИМЯ ПО УМОЛЧАНИЮ
// ============================================================
const char* DEFAULT_NAME = "mesa_6";

// ============================================================
// ТАЙМИНГИ (мс)
// ============================================================
const unsigned long DEBOUNCE_DELAY      = 50;
const unsigned long HINT_DURATION       = 2000;
const unsigned long DISPLAY_UPDATE_MS   = 500;
const unsigned long SEND_CHECK_MS       = 30000;
const unsigned long WIFI_RETRY_DELAY    = 5000;
const unsigned long WIFI_TIMEOUT_MS     = 20000;
const unsigned long TIME_SYNC_RETRY_MS  = 30000;
const unsigned long CHECKPOINT_MS       = 60000;

// ============================================================
// ОЧЕРЕДЬ
// ============================================================
#define MAX_QUEUE       20
#define QUEUE_WARNING   15

// ============================================================
// ГЛОБАЛЬНЫЕ ОБЪЕКТЫ
// ============================================================
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Preferences prefs;
String deviceMac;
String deviceName;
String bootId;

// Кэш очереди в RAM
int queueHead = 0;
int queueCount = 0;

// Флаг успешной инициализации OLED
bool oledReady = false;

// ============================================================
// ИСТОЧНИК ВРЕМЕНИ
// ============================================================
enum TimeSource { TS_NONE = 0, TS_SENSOR_UPTIME, TS_GATEWAY_UPTIME, TS_NTP };

// ============================================================
// СОСТОЯНИЕ СМЕНЫ
// ============================================================
struct ShiftState {
  bool isMachineOn = false;
  int shiftNumber = 1;
  int pressCount = 0;
  unsigned long startMs = 0;
  unsigned long endMs = 0;
  char startTimeStr[10];
  char endTimeStr[10];
} currentShift;

// ============================================================
// РАЗБЛОКИРОВКА I2C ШИНЫ (v3.6)
// ============================================================
// После ESP.restart() линии SDA/SCL могут остаться в LOW,
// что блокирует I2C. Эта функция генерирует 9 тактов на SCL,
// чтобы сбросить состояние шины и сформировать условие STOP.
void i2cUnlock() {
  pinMode(OLED_SDA, INPUT_PULLUP);
  pinMode(OLED_SCL, OUTPUT);
  
  // 9 тактов — достаточно для сброса любого I2C-устройства
  for (int i = 0; i < 9; i++) {
    digitalWrite(OLED_SCL, HIGH);
    delayMicroseconds(5);
    digitalWrite(OLED_SCL, LOW);
    delayMicroseconds(5);
  }
  
  // Условие STOP: SDA LOW→HIGH при SCL=HIGH
  pinMode(OLED_SDA, OUTPUT);
  digitalWrite(OLED_SDA, LOW);
  digitalWrite(OLED_SCL, HIGH);
  delayMicroseconds(5);
  digitalWrite(OLED_SDA, HIGH);
  delayMicroseconds(5);
  
  // Освобождаем пины для Wire.begin()
  pinMode(OLED_SDA, INPUT);
  pinMode(OLED_SCL, INPUT);
  delay(10);
}

// ============================================================
// ИНИЦИАЛИЗАЦИЯ OLED С ПОВТОРНЫМИ ПОПЫТКАМИ (v3.6)
// ============================================================
bool initOLED() {
  for (int attempt = 1; attempt <= 3; attempt++) {
    i2cUnlock();
    Wire.begin(OLED_SDA, OLED_SCL);
    Wire.setClock(400000); // 400 kHz — стабильная скорость
    delay(50);
    
    if (display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
      Serial.printf("OLED OK (attempt %d)\n", attempt);
      return true;
    }
    
    Serial.printf("OLED init failed, retry %d/3...\n", attempt);
    delay(200);
  }
  
  Serial.println("ERR OLED: display not found after 3 attempts");
  return false;
}

// ============================================================
// УПРАВЛЕНИЕ ВРЕМЕНЕМ
// ============================================================
struct TimeManager {
  bool isSynced = false;
  TimeSource source = TS_NONE;
  int startHour = 0, startMinute = 0, startSecond = 0;
  unsigned long syncMs = 0;

  bool parseTimeResponse(const String& r, int& h, int& m, int& s) {
    int hp = r.indexOf("\"hour\":");
    int mp = r.indexOf("\"minute\":");
    int sp = r.indexOf("\"second\":");
    if (hp == -1 || mp == -1 || sp == -1) return false;
    
    hp += 7; int he = r.indexOf(",", hp); if (he == -1) he = r.indexOf("}", hp);
    h = r.substring(hp, he).toInt();
    mp += 9; int me = r.indexOf(",", mp); if (me == -1) me = r.indexOf("}", mp);
    m = r.substring(mp, me).toInt();
    sp += 9; int se = r.indexOf(",", sp); if (se == -1) se = r.indexOf("}", sp);
    s = r.substring(sp, se).toInt();
    return true;
  }

  bool syncFromGateway() {
    if (WiFi.status() != WL_CONNECTED) return false;
    HTTPClient http;
    http.begin(String(GATEWAY_URL) + "/time");
    http.setTimeout(1500);
    yield();
    int code = http.GET();
    yield();
    if (code != 200) { http.end(); return false; }
    
    String response = http.getString();
    http.end();
    
    int h, m, s;
    if (!parseTimeResponse(response, h, m, s)) return false;
    
    startHour = h; startMinute = m; startSecond = s;
    syncMs = millis();
    isSynced = true;
    source = TS_GATEWAY_UPTIME;
    return true;
  }

  void getCurrent(char* buf, size_t len) {
    if (isSynced) {
      unsigned long el = (millis() - syncMs) / 1000;
      int total = (startHour * 3600 + startMinute * 60 + startSecond + el) % 86400;
      snprintf(buf, len, "%02d:%02d:%02d", total / 3600, (total % 3600) / 60, total % 60);
    } else {
      unsigned long s = millis() / 1000;
      snprintf(buf, len, "%02d:%02d:%02d", (s / 3600) % 24, (s % 3600) / 60, s % 60);
    }
  }

  const char* getSourceStr() {
    switch (source) {
      case TS_GATEWAY_UPTIME: return "gateway";
      case TS_NTP:            return "ntp";
      default:                return "uptime";
    }
  }

  void begin() { if (WiFi.status() == WL_CONNECTED) syncFromGateway(); }
  
  void periodicSync() {
    if (isSynced) return;
    static unsigned long last = 0;
    if (millis() - last > TIME_SYNC_RETRY_MS) {
      last = millis();
      if (WiFi.status() == WL_CONNECTED) syncFromGateway();
    }
  }
  
  void resetSync() { isSynced = false; source = TS_NONE; }
} timeManager;

// ============================================================
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
// ============================================================
String getMacAddress() {
  WiFi.mode(WIFI_STA); delay(50);
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  if (mac == "000000000000") {
    uint64_t id = ESP.getEfuseMac();
    char b[13];
    snprintf(b, sizeof(b), "%012llX", id);
    mac = String(b);
    if (mac.length() > 12) mac = mac.substring(mac.length() - 12);
  }
  return mac;
}

void loadDeviceName() {
  deviceName = prefs.getString("dev_name", "");
  if (deviceName.length() == 0) {
    deviceName = DEFAULT_NAME;
    prefs.putString("dev_name", deviceName);
  }
}

String generateBootId() {
  uint32_t rnd = esp_random();
  char b[9];
  snprintf(b, sizeof(b), "%08X", rnd);
  return String(b);
}

void loadQueueState() {
  queueHead = prefs.getInt("q_head", 0);
  queueCount = prefs.getInt("q_count", 0);
  if (queueHead < 0 || queueHead >= MAX_QUEUE) queueHead = 0;
  if (queueCount < 0 || queueCount > MAX_QUEUE) queueCount = 0;
}

void saveQueueState() {
  prefs.putInt("q_head", queueHead);
  prefs.putInt("q_count", queueCount);
}

// ============================================================
// ОЧЕРЕДЬ (КОЛЬЦЕВОЙ БУФЕР)
// ============================================================
void saveShiftToQueue() {
  if (queueCount >= MAX_QUEUE) {
    Serial.println("WARN: QUEUE FULL");
    return;
  }
  int writeIdx = (queueHead + queueCount) % MAX_QUEUE;
  char key[8];
  snprintf(key, sizeof(key), "q_%d", writeIdx);
  
  unsigned long durS = (currentShift.endMs - currentShift.startMs) / 1000;
  char payload[256];
  snprintf(payload, sizeof(payload),
    "mac=%s&name=%s&actions=%d&start=%s&end=%s&time_source=%s&event_duration_s=%lu&sensor_uptime_s=%lu&boot_id=%s",
    deviceMac.c_str(), deviceName.c_str(), currentShift.pressCount,
    currentShift.startTimeStr, currentShift.endTimeStr,
    timeManager.getSourceStr(), durS, millis() / 1000, bootId.c_str());
    
  prefs.putString(key, payload);
  queueCount++;
  saveQueueState();
  Serial.printf("SAVE: shift #%d -> queue (%d/%d)\n", currentShift.shiftNumber, queueCount, MAX_QUEUE);
}

void processSendQueue() {
  if (queueCount == 0 || WiFi.status() != WL_CONNECTED) return;
  
  char key[8];
  snprintf(key, sizeof(key), "q_%d", queueHead);
  String payload = prefs.getString(key, "");
  
  if (payload.length() == 0) {
    prefs.remove(key);
    queueHead = (queueHead + 1) % MAX_QUEUE;
    queueCount--;
    saveQueueState();
    return;
  }
  
  HTTPClient http;
  http.begin(String(GATEWAY_URL) + "/data");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.setTimeout(1500);
  yield();
  int code = http.POST(payload);
  yield();
  http.end();
  
  if (code == 200) {
    prefs.remove(key);
    queueHead = (queueHead + 1) % MAX_QUEUE;
    queueCount--;
    saveQueueState();
    Serial.printf("SEND OK (%d/%d)\n", queueCount, MAX_QUEUE);
  } else {
    Serial.printf("SEND FAIL %d\n", code);
  }
}

void saveCheckpoint() {
  prefs.putInt("shift_num", currentShift.shiftNumber);
  prefs.putInt("press_count", currentShift.pressCount);
}

// ============================================================
// WI-FI
// ============================================================
enum WiFiState { WIFI_IDLE, WIFI_CONNECTING, WIFI_CONNECTED };
WiFiState wifiState = WIFI_IDLE;
unsigned long wifiTimer = 0;

void connectWiFiAsync() {
  switch (wifiState) {
    case WIFI_CONNECTED:
      if (WiFi.status() != WL_CONNECTED) {
        wifiState = WIFI_IDLE;
        wifiTimer = millis();
        timeManager.resetSync();
      }
      break;
    case WIFI_IDLE:
      if (millis() - wifiTimer < WIFI_RETRY_DELAY) return;
      WiFi.mode(WIFI_STA);
      WiFi.setSleep(WIFI_PS_NONE);
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASS);
      wifiState = WIFI_CONNECTING;
      wifiTimer = millis();
      break;
    case WIFI_CONNECTING:
      if (WiFi.status() == WL_CONNECTED) {
        wifiState = WIFI_CONNECTED;
        timeManager.begin();
      } else if (millis() - wifiTimer > WIFI_TIMEOUT_MS) {
        wifiState = WIFI_IDLE;
        wifiTimer = millis();
      }
      break;
  }
}

void sendStatusSignal(const char* status) {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  http.begin(String(GATEWAY_URL) + "/status");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.setTimeout(1500);
  
  char data[96];
  snprintf(data, sizeof(data), "nombre=%s&status=%s", deviceName.c_str(), status);
  yield();
  int code = http.POST(data);
  yield();
  http.end();
  
  if (code == 200) Serial.printf("STATUS '%s' OK\n", status);
}

// ============================================================
// ДИСПЛЕЙ
// ============================================================
void drawWifiIcon(int x, int y, bool connected) {
  int cx = x + 8, cy = y + 12;
  if (connected) display.fillCircle(cx, cy, 2, SSD1306_WHITE);
  else           display.drawCircle(cx, cy, 2, SSD1306_WHITE);
  display.drawCircleHelper(cx, cy, 5,  3, SSD1306_WHITE);
  display.drawCircleHelper(cx, cy, 8,  3, SSD1306_WHITE);
  display.drawCircleHelper(cx, cy, 11, 3, SSD1306_WHITE);
}

void drawPowerIcon(int cx, int cy, int r) {
  display.drawCircle(cx, cy, r, SSD1306_WHITE);
  display.fillRect(cx - r - 2, cy - r - 2, (r + 2) * 2, r + 4, SSD1306_BLACK);
  for (int a = 0; a <= 180; a += 5) {
    float rad = radians(a);
    display.drawPixel(cx + r * cos(rad), cy + r * sin(rad), SSD1306_WHITE);
  }
  display.drawLine(cx, cy - r - 6, cx, cy - r + 4, SSD1306_WHITE);
}

void drawHeader(const char* status) {
  drawWifiIcon(0, 0, WiFi.status() == WL_CONNECTED);
  display.setTextSize(2);
  display.setCursor(30, 0);
  display.print(status);
  
  display.setTextSize(1);
  display.setCursor(108, 0);
  if (queueCount == 0) {
    display.print("v");
  } else if (queueCount >= MAX_QUEUE) {
    if ((millis() / 300) % 2 == 0) { display.print(queueCount); display.print("!"); }
  } else if (queueCount >= QUEUE_WARNING) {
    display.print(queueCount); display.print("~");
  } else {
    display.print(queueCount);
  }
  display.drawLine(0, 16, 128, 16, SSD1306_WHITE);
}

bool showHintActive = false;
unsigned long hintStartTime = 0;

void updateDisplay() {
  if (!oledReady) return; // v3.6: защита от вызова при нерабочем OLED
  
  if (showHintActive) {
    if (millis() - hintStartTime < HINT_DURATION) return;
    showHintActive = false;
  }
  
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  int16_t x1, y1; uint16_t w, h;
  
  if (currentShift.isMachineOn) {
    // ===== ACTIVO =====
    drawHeader("ACTIVO");
    display.setTextSize(3);
    char countStr[12];
    snprintf(countStr, sizeof(countStr), "%d", currentShift.pressCount);
    display.getTextBounds(countStr, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((128 - w) / 2, 20);
    display.print(countStr);
    
    display.setTextSize(2);
    unsigned long el = (millis() - currentShift.startMs) / 1000;
    char timeStr[10];
    snprintf(timeStr, sizeof(timeStr), "%02lu:%02lu", el / 60, el % 60);
    display.setCursor(5, 46);
    display.print(timeStr);
  } else {
    // ===== PASIVO =====
    drawHeader("PASIVO");
    drawPowerIcon(64, 36, 12);
    
    display.setTextSize(1);
    display.setCursor(2, 54);
    display.print("Smena:");
    char shiftStr[8];
    snprintf(shiftStr, sizeof(shiftStr), "#%d", currentShift.shiftNumber);
    display.getTextBounds(shiftStr, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(126 - w, 54);
    display.print(shiftStr);
  }
  display.display();
}

void showMessage(const char* msg) {
  if (!oledReady) return; // v3.6: защита
  
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  int16_t x1, y1; uint16_t w, h;
  
  display.setTextSize(2);
  display.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);
  if (w > 126) {
    display.setTextSize(1);
    display.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);
  }
  display.setCursor((128 - w) / 2, 8);
  display.print(msg);
  drawPowerIcon(64, 44, 12);
  display.display();
  
  showHintActive = true;
  hintStartTime = millis();
}

// ============================================================
// КНОПКИ
// ============================================================
bool lastBtnCount = HIGH, stateBtnCount = HIGH;
unsigned long lastDebounceCount = 0;

bool lastShiftBtnState = HIGH, shiftBtnState = HIGH;
unsigned long lastDebounceShift = 0;

bool lastRebootBtnState = HIGH, rebootBtnState = HIGH;
unsigned long lastDebounceReboot = 0;

void handleButtons() {
  // ---- COUNT ----
  bool rc = digitalRead(BUTTON_COUNT);
  if (rc != lastBtnCount) lastDebounceCount = millis();
  if ((millis() - lastDebounceCount) > DEBOUNCE_DELAY) {
    if (rc != stateBtnCount) {
      stateBtnCount = rc;
      if (stateBtnCount == LOW) {
        if (currentShift.isMachineOn) {
          currentShift.pressCount++;
          if (currentShift.pressCount % 10 == 0) saveCheckpoint();
        } else {
          showMessage(deviceName.c_str());
        }
      }
    }
  }
  lastBtnCount = rc;

  // ---- SHIFT ----
  bool rs = digitalRead(BUTTON_SHIFT);
  if (rs != lastShiftBtnState) lastDebounceShift = millis();
  if ((millis() - lastDebounceShift) > DEBOUNCE_DELAY) {
    if (rs != shiftBtnState) {
      shiftBtnState = rs;
      showHintActive = false;
      if (shiftBtnState == LOW) {
        currentShift.isMachineOn = true;
        currentShift.startMs = millis();
        timeManager.getCurrent(currentShift.startTimeStr, sizeof(currentShift.startTimeStr));
        currentShift.pressCount = 0;
        saveCheckpoint();
        sendStatusSignal("abrir");
        Serial.println("=== ACTIVO ===");
      } else {
        currentShift.isMachineOn = false;
        currentShift.endMs = millis();
        timeManager.getCurrent(currentShift.endTimeStr, sizeof(currentShift.endTimeStr));
        sendStatusSignal("cerrado");
        Serial.println("=== PASIVO ===");
        saveShiftToQueue();
        currentShift.shiftNumber++;
        currentShift.pressCount = 0;
        saveCheckpoint();
      }
    }
  }
  lastShiftBtnState = rs;

  // ---- REBOOT (GPIO9) ----
  bool rr = digitalRead(BUTTON_REBOOT);
  if (rr != lastRebootBtnState) lastDebounceReboot = millis();
  if ((millis() - lastDebounceReboot) > DEBOUNCE_DELAY) {
    if (rr != rebootBtnState) {
      rebootBtnState = rr;
      if (rebootBtnState == LOW) {
        Serial.println("HW REBOOT triggered by button on GPIO9...");
        if (currentShift.isMachineOn) saveCheckpoint();
        delay(100);
        ESP.restart();
      }
    }
  }
  lastRebootBtnState = rr;
}

// ============================================================
// SERIAL КОМАНДЫ
// ============================================================
void clearQueue() {
  for (int i = 0; i < MAX_QUEUE; i++) {
    char k[8]; snprintf(k, sizeof(k), "q_%d", i);
    prefs.remove(k);
  }
  queueHead = 0; queueCount = 0;
  saveQueueState();
  Serial.println("OK: queue cleared");
}

void resetShiftCount() {
  currentShift.shiftNumber = 1;
  currentShift.pressCount = 0;
  saveCheckpoint();
  Serial.println("OK: shift counter reset to #1");
}

bool confirmYes(const char* prompt, unsigned long ms) {
  Serial.printf("WARN: %s — type 'yes' within %lu sec\n", prompt, ms / 1000);
  unsigned long t = millis() + ms;
  while (millis() < t) {
    if (Serial.available()) {
      String c = Serial.readStringUntil('\n');
      c.trim();
      if (c == "yes") return true;
      return false;
    }
    delay(10);
  }
  return false;
}

void handleSerialCommands() {
  if (!Serial.available()) return;
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  
  if (cmd.startsWith("setname:")) {
    String n = cmd.substring(8); n.trim();
    if (n.length() > 0) {
      prefs.putString("dev_name", n);
      deviceName = n;
      Serial.println("OK name: " + deviceName);
    }
  }
  else if (cmd == "info") {
    char timeBuf[10];
    timeManager.getCurrent(timeBuf, sizeof(timeBuf));
    Serial.println("\n=== INFO ===");
    Serial.printf("FW Version: %s\n", FIRMWARE_VERSION);
    Serial.printf("Name: %s | MAC: %s\n", deviceName.c_str(), deviceMac.c_str());
    Serial.printf("Boot ID: %s\n", bootId.c_str());
    Serial.printf("Shift: #%d | Actions: %d\n", currentShift.shiftNumber, currentShift.pressCount);
    Serial.printf("Status: %s\n", currentShift.isMachineOn ? "ACTIVO" : "PASIVO");
    Serial.printf("Queue: %d/%d (head=%d)\n", queueCount, MAX_QUEUE, queueHead);
    Serial.printf("WiFi: %s\n", WiFi.status() == WL_CONNECTED ? "OK" : "NO");
    Serial.printf("Time: %s (%s)\n", timeBuf, timeManager.getSourceStr());
    Serial.printf("Uptime: %lu s\n", millis() / 1000);
    Serial.printf("OLED: %s\n", oledReady ? "OK" : "FAIL");
    Serial.println("============");
  }
  else if (cmd == "send") {
    if (WiFi.status() == WL_CONNECTED) processSendQueue();
    else Serial.println("WARN: no WiFi");
  }
  else if (cmd == "clear") {
    if (confirmYes("clear queue", 5000)) clearQueue();
    else Serial.println("CANCEL");
  }
  else if (cmd == "resetshift") {
    if (confirmYes("reset shift counter", 5000)) resetShiftCount();
    else Serial.println("CANCEL");
  }
  else if (cmd == "sync") {
    Serial.println("SYNC time...");
    timeManager.resetSync();
    timeManager.begin();
  }
  else if (cmd == "reboot") {
    Serial.println("SW REBOOT...");
    if (currentShift.isMachineOn) saveCheckpoint();
    delay(500);
    ESP.restart();
  }
  else if (cmd == "help") {
    Serial.println("\n=== COMMANDS ===");
    Serial.printf("FW: %s\n", FIRMWARE_VERSION);
    Serial.println("setname:NAME   — set device name");
    Serial.println("info           — show state");
    Serial.println("send           — send one queue item now");
    Serial.println("clear          — clear queue (needs 'yes')");
    Serial.println("resetshift     — reset shift counter (needs 'yes')");
    Serial.println("sync           — resync time from gateway");
    Serial.println("reboot         — restart device (software)");
    Serial.println("help           — this list");
    Serial.println("=================");
  }
}

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("\n============================================");
  Serial.printf("  ESP32-C6 ДАТЧИК СТАНКА %s\n", FIRMWARE_VERSION);
  Serial.println("  (Minimized Display + Anti-Hang + HW Reset)");
  Serial.println("  (I2C Auto-Recovery after ESP.restart)");
  Serial.println("============================================");
  
  pinMode(BUTTON_COUNT, INPUT_PULLUP);
  pinMode(BUTTON_SHIFT, INPUT_PULLUP);
  pinMode(BUTTON_REBOOT, INPUT_PULLUP);
  
  bool initCount = digitalRead(BUTTON_COUNT);
  lastBtnCount = initCount;
  stateBtnCount = initCount;
  
  bool initReboot = digitalRead(BUTTON_REBOOT);
  lastRebootBtnState = initReboot;
  rebootBtnState = initReboot;

  // ============================================================
  // v3.6: ИНИЦИАЛИЗАЦИЯ OLED С РАЗБЛОКИРОВКОЙ I2C
  // ============================================================
  Serial.println("Init OLED with I2C unlock...");
  oledReady = initOLED();
  
  if (oledReady) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("BOOTING ");
    display.print(FIRMWARE_VERSION);
    display.print("...");
    display.display();
  } else {
    Serial.println("WARN: running without OLED display");
  }
  
  prefs.begin("mesa6_data", false);
  currentShift.shiftNumber = prefs.getInt("shift_num", 1);
  currentShift.pressCount = prefs.getInt("press_count", 0);
  loadQueueState();
  
  deviceMac = getMacAddress();
  bootId = generateBootId();
  loadDeviceName();
  
  WiFi.setSleep(WIFI_PS_NONE);
  connectWiFiAsync();
  
  unsigned long t = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t < 10000) delay(100);
  timeManager.begin();
  
  bool initShift = digitalRead(BUTTON_SHIFT);
  lastShiftBtnState = initShift;
  shiftBtnState = initShift;
  
  if (initShift == LOW) {
    currentShift.isMachineOn = true;
    currentShift.startMs = millis();
    timeManager.getCurrent(currentShift.startTimeStr, sizeof(currentShift.startTimeStr));
    sendStatusSignal("abrir");
  } else {
    currentShift.isMachineOn = false;
    sendStatusSignal("cerrado");
  }
  
  updateDisplay();
  Serial.println("OK ready 24/7");
  Serial.println("Type 'help' for commands");
}

// ============================================================
// LOOP
// ============================================================
void loop() {
  handleButtons();
  
  static unsigned long lastDisplay = 0;
  if (millis() - lastDisplay > DISPLAY_UPDATE_MS) {
    lastDisplay = millis();
    updateDisplay();
  }
  
  connectWiFiAsync();
  
  static unsigned long lastSend = 0;
  if (millis() - lastSend > SEND_CHECK_MS) {
    lastSend = millis();
    processSendQueue();
  }
  
  timeManager.periodicSync();
  
  static unsigned long lastChk = 0;
  if (millis() - lastChk > CHECKPOINT_MS) {
    lastChk = millis();
    if (currentShift.isMachineOn) saveCheckpoint();
  }
  
  handleSerialCommands();
  delay(5);
}