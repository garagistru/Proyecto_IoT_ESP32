import serial
import time

# ЗАЧЕМ: Указываем правильный порт, который мы нашли через dmesg
PORT = '/dev/ttyACM0'  
BAUDRATE = 115200 # Должна совпадать с Serial.begin(115200) в коде ESP32

print(f"🚀 Попытка открытия порта {PORT}...")

try:
    # ЗАЧЕМ: Открываем соединение. timeout=1 защищает от вечного зависания.
    ser = serial.Serial(PORT, BAUDRATE, timeout=1)
    print("✅ Порт успешно открыт! Ожидаю данные от ESP32-S3...")
    print("💡 Подсказка: Нажми кнопку RST на плате ESP32, чтобы увидеть загрузочный лог.")
    print("Нажми Ctrl+C для выхода.\n")
    
    # ЗАЧЕМ: Даем плате 2 секунды на перезагрузку после открытия порта
    time.sleep(2)
    
    while True:
        # ЗАЧЕМ: Проверяем, есть ли данные в буфере
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').strip()
            if line:
                print(f"📥 Получено: {line}")
                
except serial.SerialException as e:
    print(f"❌ Ошибка открытия порта: {e}")
except KeyboardInterrupt:
    print("\n🛑 Тест остановлен пользователем.")
finally:
    # ЗАЧЕМ: Гарантированно закрываем порт при выходе
    if 'ser' in locals() and ser.is_open:
        ser.close()
        print("✅ Порт закрыт.")
