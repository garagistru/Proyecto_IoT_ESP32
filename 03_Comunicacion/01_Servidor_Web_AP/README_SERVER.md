# 🐧 Настройка сервера Ubuntu для EnrollaDatos

## 📋 Описание

Документация по настройке сервера Ubuntu 22.04 для приёма, хранения и отображения данных от системы EnrollaDatos.

---

## 📁 Структура сервера

### Папки и файлы

```text
/home/don/
├── Proyecto_IoT_Servidor/
│   ├── src/
│   │   └── server.py          # Основной скрипт приёма данных
│   ├── web/
│   │   └── index.html         # Веб-интерфейс
│   ├── logs/
│   │   └── server.log         # Лог работы сервера
│   ├── web_server.py          # Flask-сервер для веб-интерфейса
│   └── cleanup.sh             # Скрипт очистки данных
├── data/
│   ├── microclima_1/
│   │   └── 2026-08-26.json    # Данные по дням
│   ├── mesa_6/
│   │   └── 2026-08-26.json    # Данные по дням
│   └── .../
└── logs/
    └── server.log             # Старый лог (удалён)



    🔧 Настройка постоянной ссылки на порт
Создание ссылки /dev/ttyESP32
bash
# Проверяем порт
ls -l /dev/ttyACM*

# Создаём ссылку
sudo ln -s /dev/ttyACM0 /dev/ttyESP32

# Проверяем
ls -l /dev/ttyESP32
🐍 Настройка Python-сервера
Зависимости
bash
pip3 install pyserial flask flask-cors
Файл src/server.py
Основной скрипт:

Принимает данные с /dev/ttyESP32

Парсит JSON и текстовые данные

Сохраняет по датчикам в /home/don/data/<sensor>/<date>.json

Автоматически переподключается при обрыве связи

Фильтрует мусорные данные (temp от -20 до 50°C)

Путь: /home/don/Proyecto_IoT_Servidor/src/server.py

🔄 Автозапуск через systemd
Файл службы
Путь: /etc/systemd/system/enrolla-datos.service

ini
[Unit]
Description=EnrollaDatos Data Server
After=multi-user.target

[Service]
Type=simple
User=don
WorkingDirectory=/home/don/Proyecto_IoT_Servidor
ExecStart=/usr/bin/python3 /home/don/Proyecto_IoT_Servidor/src/server.py
Restart=always
RestartSec=10
StandardOutput=append:/home/don/Proyecto_IoT_Servidor/logs/server.log
StandardError=append:/home/don/Proyecto_IoT_Servidor/logs/server.log

[Install]
WantedBy=multi-user.target
Команды управления
bash
sudo systemctl daemon-reload
sudo systemctl enable enrolla-datos.service
sudo systemctl start enrolla-datos.service
sudo systemctl status enrolla-datos.service

# Просмотр логов
sudo journalctl -u enrolla-datos.service -f
🗂️ Хранение данных
Структура
text
~/data/
├── <sensor_name>/
│   ├── 2026-08-26.json
│   ├── 2026-08-27.json
│   └── ...
└── ...
Формат данных
Для датчиков температуры:

json
{
  "sensor": "microclima_1",
  "type": "sensor_data",
  "data": {
    "temperature": 22.5,
    "humidity": 58.0,
    "timestamp": "2026-08-26T12:00:00"
  }
}
Для станков (mesa_6):

json
{
  "sensor": "mesa_6",
  "type": "shift_data",
  "data": {
    "shift_number": 232,
    "actions": 8,
    "timestamp": "2026-08-26T12:00:00"
  }
}
🧹 Очистка данных
Скрипт cleanup.sh
bash
#!/bin/bash
DATA_FILE="/home/don/data/sensor_data.json"
MAX_RECORDS=500

if [ -f "$DATA_FILE" ]; then
    tail -n "$MAX_RECORDS" "$DATA_FILE" > "$DATA_FILE.tmp"
    mv "$DATA_FILE.tmp" "$DATA_FILE"
fi
Добавление в cron
bash
crontab -e
# Добавить:
0 * * * * /home/don/Proyecto_IoT_Servidor/cleanup.sh
```
