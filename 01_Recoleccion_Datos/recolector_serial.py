import serial
import sqlite3
import time
import re # ЗАЧЕМ: Библиотека для поиска шаблонов в тексте (регулярные выражения)

# === НАСТРОЙКИ ===
PORT = '/dev/ttyACM0'
BAUDRATE = 115200
# Используем абсолютный путь к базе данных, чтобы скрипт работал из любой папки
DB_FILE = "/home/don/Proyecto_IoT_Servidor/01_Recoleccion_Datos/sensores.db"

# Регулярное выражение для поиска строки вида: 
# "📊 Показание от dht11_nodemcu_garage: Temp=22.20°C, Hum=67.00%"
# ЗАЧЕМ: Оно автоматически вытащит имя датчика, температуру и влажность в отдельные переменные.
DATA_PATTERN = re.compile(r"Показание от (.*?): Temp=([\d.]+)°C, Hum=([\d.]+)%")

def inicializar_db():
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS lecturas (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            nombre_sensor TEXT,
            temperatura REAL,
            humedad REAL,
            fecha_registro TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    ''')
    conn.commit()
    conn.close()
    print("✅ База данных инициализирована.")

print(f"🚀 Запуск сборщика данных через Serial ({PORT})...")
inicializar_db()

try:
    ser = serial.Serial(PORT, BAUDRATE, timeout=1)
    print("✅ Порт открыт. Ожидаю данные от датчиков...\n")
    time.sleep(2) # Даем ESP32 время на стабилизацию
    
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            
            # ЗАЧЕМ: Проверяем, соответствует ли строка нашему шаблону с данными
            match = DATA_PATTERN.search(line)
            if match:
                # Извлекаем группы, найденные регулярным выражением
                nombre = match.group(1)
                temp = float(match.group(2))
                hum = float(match.group(3))
                
                print(f"📥 Найдены данные: [{nombre}] T: {temp}°C, H: {hum}%")
                
                # ЗАЧЕМ: Сохраняем в базу данных
                conn = sqlite3.connect(DB_FILE)
                cursor = conn.cursor()
                cursor.execute('''
                    INSERT INTO lecturas (nombre_sensor, temperatura, humedad)
                    VALUES (?, ?, ?)
                ''', (nombre, temp, hum))
                conn.commit()
                conn.close()
                print("   💾 Успешно сохранено в БД!\n")
                
except serial.SerialException as e:
    print(f"❌ Ошибка порта: {e}")
except KeyboardInterrupt:
    print("\n🛑 Сборщик данных остановлен пользователем.")
finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()
        print("✅ Порт закрыт.")
