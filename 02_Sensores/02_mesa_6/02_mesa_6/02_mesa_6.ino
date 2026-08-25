#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ===== НАСТРОЙКИ =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C
#define OLED_SDA 19
#define OLED_SCL 20

#define BUTTON_COUNT 14
#define BUTTON_SHIFT 18

const char* DEVICE_NAME = "mesa_6";
const char* WIFI_SSID = "Mechanic";
const char* WIFI_PASS = "12345678";
const char* SERVER_URL = "http://192.168.4.1";

// ===== ГЛОБАЛЬНЫЕ ОБЪЕКТЫ =====
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Preferences prefs;

// ===== СОСТОЯНИЕ СИСТЕМЫ =====
struct ShiftState {
    int shiftNumber = 1;
    bool isMachineOn = false;
    uint32_t startTime = 0;
    int pressCount = 0;
} currentShift;

// Кнопки
bool lastBtnCount = HIGH;
bool stateBtnCount = HIGH;
unsigned long lastDebounceCount = 0;

bool lastShiftButton = HIGH; // 🆕 Переименовал для ясности
unsigned long lastDebounceShift = 0;
const unsigned long debounceDelay = 50;

unsigned long lastDisplayUpdate = 0;
unsigned long lastSendCheck = 0;

// ===== ПРОТОТИПЫ =====
void initHardware();
void connectWiFi();
void updateDisplay();
void drawWifiIcon(int x, int y, bool connected);
void showMessage(const char* msg);
void saveCheckpoint();
void processSendQueue();
void sendStatusSignal(const char* status);

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== ЗАПУСК mesa_6 ===");

    initHardware();

    prefs.begin("mesa6_data", false);
    currentShift.shiftNumber = prefs.getInt("shift_num", 1);
    currentShift.pressCount = prefs.getInt("press_count", 0);

    connectWiFi();

    // Проверяем физическое состояние кнопки смены
    bool initialShiftState = digitalRead(BUTTON_SHIFT);
    lastShiftButton = initialShiftState; // 🆕 Запоминаем начальное состояние
    
    Serial.printf(" Начальное состояние кнопки смены: %s\n", 
                  initialShiftState == LOW ? "ЗАМКНУТА (LOW)" : "РАЗОМКНУТА (HIGH)");

    if (initialShiftState == LOW) {
        currentShift.isMachineOn = true;
        currentShift.startTime = millis();
        sendStatusSignal("abrir");
        Serial.println("🔧 Восстановление: смена открыта");
    } else {
        currentShift.isMachineOn = false;
        sendStatusSignal("cerrado");
        Serial.println(" Восстановление: смена закрыта");
    }
    
    updateDisplay();
    Serial.println("✅ Система готова!");
    Serial.printf(" isMachineOn = %s\n", currentShift.isMachineOn ? "TRUE" : "FALSE");
}

void loop() {
    // 1. Обработка кнопки счетчика
    bool readingCount = digitalRead(BUTTON_COUNT);
    if (readingCount != lastBtnCount) lastDebounceCount = millis();
    if ((millis() - lastDebounceCount) > debounceDelay) {
        if (readingCount != stateBtnCount) {
            stateBtnCount = readingCount;
            if (stateBtnCount == LOW) {
                Serial.printf("🔍 К1 нажата, isMachineOn = %s\n", 
                              currentShift.isMachineOn ? "TRUE" : "FALSE");
                if (currentShift.isMachineOn) {
                    currentShift.pressCount++;
                    Serial.printf("[К1] Действие: %d\n", currentShift.pressCount);
                    if (currentShift.pressCount % 10 == 0) saveCheckpoint();
                } else {
                    showMessage(DEVICE_NAME);
                }
            }
        }
    }
    lastBtnCount = readingCount;

    // 2.  УПРОЩЕННАЯ обработка кнопки смены с фиксацией
    bool currentShiftButton = digitalRead(BUTTON_SHIFT);
    
    if (currentShiftButton != lastShiftButton) {
        delay(50); // Простой debounce
        currentShiftButton = digitalRead(BUTTON_SHIFT);
        
        if (currentShiftButton != lastShiftButton) {
            lastShiftButton = currentShiftButton;
            
            Serial.printf("🔄 Переход кнопки смены: %s → %s\n",
                          lastShiftButton == LOW ? "LOW" : "HIGH",
                          currentShiftButton == LOW ? "LOW" : "HIGH");
            
            if (currentShiftButton == LOW) {
                // Кнопка ЗАМКНУТА - станок включен
                currentShift.isMachineOn = true;
                currentShift.startTime = millis();
                currentShift.pressCount = 0;
                saveCheckpoint();
                sendStatusSignal("abrir");
                Serial.println("=== СТАНОК ВКЛЮЧЕН (abrir) ===");
            } else {
                // Кнопка РАЗОМКНУТА - станок выключен
                currentShift.isMachineOn = false;
                sendStatusSignal("cerrado");
                Serial.println("=== СТАНОК ВЫКЛЮЧЕН (cerrado) ===");
                
                // Сохраняем данные смены
                int qCount = prefs.getInt("q_count", 0);
                if (qCount < 10) {
                    String payload = "start=" + String(currentShift.startTime) + 
                                     "&end=" + String(millis()) + 
                                     "&actions=" + String(currentShift.pressCount);
                    String key = "q_" + String(qCount);
                    prefs.putString(key.c_str(), payload);
                    prefs.putInt("q_count", qCount + 1);
                }
                
                currentShift.shiftNumber++;
                currentShift.pressCount = 0;
                saveCheckpoint();
            }
            
            Serial.printf("📊 isMachineOn теперь = %s\n", 
                          currentShift.isMachineOn ? "TRUE" : "FALSE");
        }
    }

    // 3. Обновление дисплея
    if (millis() - lastDisplayUpdate > 500) {
        lastDisplayUpdate = millis();
        updateDisplay();
    }

    // 4. Фоновая отправка
    if (millis() - lastSendCheck > 15000) {
        lastSendCheck = millis();
        if (WiFi.status() == WL_CONNECTED) processSendQueue();
        else connectWiFi();
    }
}

// ===== ОСТАЛЬНЫЕ ФУНКЦИИ БЕЗ ИЗМЕНЕНИЙ =====

void sendStatusSignal(const char* status) {
    if (WiFi.status() != WL_CONNECTED) return;
    
    HTTPClient http;
    String url = String(SERVER_URL) + "/status";
    http.begin(url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
    String postData = "nombre=" + String(DEVICE_NAME) + "&status=" + String(status);
    int code = http.POST(postData);
    
    if (code == 200) {
        Serial.printf("📡 Сигнал '%s' отправлен!\n", status);
    } else {
        Serial.printf("⚠️ Ошибка отправки статуса: %d\n", code);
    }
    http.end();
}

void initHardware() {
    Wire.begin(OLED_SDA, OLED_SCL);
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println("❌ ОШИБКА: OLED не найден!");
        while (true);
    }
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    
    pinMode(BUTTON_COUNT, INPUT_PULLUP);
    pinMode(BUTTON_SHIFT, INPUT_PULLUP);
}

void connectWiFi() {
    if (WiFi.status() == WL_CONNECTED) return;
    Serial.print("Подключение к WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 15) {
        delay(300); Serial.print("."); attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println(" ПОДКЛЮЧЕНО!");
    } else {
        Serial.println(" ОШИБКА!");
    }
}

void saveCheckpoint() {
    prefs.putInt("shift_num", currentShift.shiftNumber);
    prefs.putInt("press_count", currentShift.pressCount);
}

void processSendQueue() {
    int qCount = prefs.getInt("q_count", 0);
    if (qCount == 0) return;

    Serial.printf("📤 Отправка смены (осталось: %d)...\n", qCount);
    String key0 = "q_0";
    String payloadData = prefs.getString(key0.c_str(), "");
    
    int currentShiftToSend = currentShift.shiftNumber - qCount; 
    if (currentShiftToSend < 1) currentShiftToSend = 1;

    int actions = 0;
    int actionsIndex = payloadData.indexOf("actions=");
    if (actionsIndex != -1) actions = payloadData.substring(actionsIndex + 8).toInt();

    HTTPClient http;
    String url = String(SERVER_URL) + "/data";
    http.begin(url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
    String postData = "nombre=" + String(DEVICE_NAME) + 
                      "&temperatura=" + String(currentShiftToSend) + 
                      "&humedad=" + String(actions);
    
    int code = http.POST(postData);
    if (code == 200) {
        Serial.println("✅ Успешно отправлено!");
        prefs.remove("q_0");
        for (int i = 1; i < qCount; i++) {
            String keyFrom = "q_" + String(i);
            String keyTo = "q_" + String(i - 1);
            prefs.putString(keyTo.c_str(), prefs.getString(keyFrom.c_str(), ""));
            prefs.remove(keyFrom.c_str());
        }
        prefs.putInt("q_count", qCount - 1);
    } else {
        Serial.printf("❌ Ошибка отправки: %d\n", code);
    }
    http.end();
}

void drawWifiIcon(int x, int y, bool connected) {
    int cx = x + 8;
    int cy = y + 14;
    if (connected) {
        display.fillCircle(cx, cy, 2, SSD1306_WHITE);
        display.drawCircleHelper(cx, cy, 5, 3, SSD1306_WHITE);
        display.drawCircleHelper(cx, cy, 8, 3, SSD1306_WHITE);
        display.drawCircleHelper(cx, cy, 11, 3, SSD1306_WHITE);
    } else {
        display.drawCircle(cx, cy, 2, SSD1306_WHITE);
        display.drawCircleHelper(cx, cy, 5, 3, SSD1306_WHITE);
        display.drawCircleHelper(cx, cy, 8, 3, SSD1306_WHITE);
        display.drawCircleHelper(cx, cy, 11, 3, SSD1306_WHITE);
    }
}

void updateDisplay() {
    display.clearDisplay();
    
    // ========================================
    // ЖЕЛТАЯ ЗОНА (Y=0 до Y=15) - Сеть и статус
    // ========================================
    display.setTextSize(1);
    
    // Крупная иконка Wi-Fi (часть выходит в голубую зону - двухцветный эффект!)
    drawWifiIconLarge(0, 0, WiFi.status() == WL_CONNECTED);
    
    // Статус смены крупным шрифтом
    display.setTextSize(2);
    display.setCursor(50, 2);
    if (currentShift.isMachineOn) {
        bool blink = ((millis() / 500) % 2) == 0;
        if (blink) display.print("activo");
    } else {
        display.print("pasivo");
    }

    display.drawLine(0, 15, 128, 15, SSD1306_WHITE); // Разделитель

    // ========================================
    // СИНЯЯ ЗОНА (Y=16 до Y=63) - Только символ питания
    // ========================================
    display.setTextColor(SSD1306_WHITE);
    
    if (currentShift.isMachineOn) {
        // Режим работы: счетчик и время
        display.setTextSize(3);
        String countStr = String(currentShift.pressCount);
        int16_t x1, y1;
        uint16_t w, h;
        display.getTextBounds(countStr, 0, 0, &x1, &y1, &w, &h);
        display.setCursor((128 - w) / 2, 22);
        display.println(countStr);

        display.setTextSize(2);
        unsigned long elapsed = (millis() - currentShift.startTime) / 1000;
        unsigned long m = elapsed / 60;
        unsigned long s = elapsed % 60;
        char buf[6];
        snprintf(buf, sizeof(buf), "%02lu:%02lu", m, s);
        String timeStr = String(buf);
        
        display.getTextBounds(timeStr, 0, 0, &x1, &y1, &w, &h);
        display.setCursor((128 - w) / 2, 46);
        display.println(timeStr);
    } else {
        // Режим покоя: только крупный символ питания
        drawPowerIcon(64, 40, 18); // центр X=64, Y=40, радиус 18
    }

    display.display();
}

// ========================================
// КРУПНАЯ ИКОНКА WI-FI (с двухцветным эффектом)
// ========================================
void drawWifiIconLarge(int x, int y, bool connected) {
    int cx = x + 12;  // центр X
    int cy = y + 14;  // центр Y
    
    if (connected) {
        // Центральная точка (закрашена)
        display.fillCircle(cx, cy, 3, SSD1306_WHITE);
        // Три дуги (верхние полуокружности)
        display.drawCircleHelper(cx, cy, 7, 3, SSD1306_WHITE);
        display.drawCircleHelper(cx, cy, 11, 3, SSD1306_WHITE);
        display.drawCircleHelper(cx, cy, 15, 3, SSD1306_WHITE);
    } else {
        // Только контуры
        display.drawCircle(cx, cy, 3, SSD1306_WHITE);
        display.drawCircleHelper(cx, cy, 7, 3, SSD1306_WHITE);
        display.drawCircleHelper(cx, cy, 11, 3, SSD1306_WHITE);
        display.drawCircleHelper(cx, cy, 15, 3, SSD1306_WHITE);
    }
}

// ========================================
// КРУПНЫЙ СИМВОЛ ПИТАНИЯ
// ========================================
void drawPowerIcon(int cx, int cy, int radius) {
    // Рисуем круг (неполный - с разрывом сверху)
    // Используем drawCircle и закрашиваем верхнюю часть черным
    
    // Сначала рисуем полный круг
    display.drawCircle(cx, cy, radius, SSD1306_WHITE);
    
    // Закрашиваем верхнюю часть (разрыв для символа питания)
    // Рисуем черный прямоугольник поверх верхней части круга
    display.fillRect(cx - radius - 2, cy - radius - 2, (radius + 2) * 2, radius + 4, SSD1306_BLACK);
    
    // Перерисовываем нижнюю половину круга
    for (int angle = 0; angle <= 180; angle += 5) {
        float rad = radians(angle);
        int px = cx + radius * cos(rad);
        int py = cy + radius * sin(rad);
        display.drawPixel(px, py, SSD1306_WHITE);
    }
    
    // Вертикальная линия сверху (символ питания)
    display.drawLine(cx, cy - radius - 6, cx, cy - radius + 4, SSD1306_WHITE);
}

void showMessage(const char* message) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(3);
    int16_t x1, y1; uint16_t w, h;
    display.getTextBounds(message, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((128 - w) / 2, (64 - h) / 2);
    display.println(message);
    display.display();
    delay(800);
    updateDisplay();
}