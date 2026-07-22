void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n=== Тест памяти ESP32-S3 N16R8 ===");
  
  // ЗАЧЕМ: Функция ESP.getFlashChipSize() возвращает реальный размер флеш-памяти в байтах.
  // Делим на 1024*1024, чтобы получить мегабайты — так человеку понятнее.
  uint32_t flashSize = ESP.getFlashChipSize() / (1024 * 1024);
  Serial.print("Flash память: ");
  Serial.print(flashSize);
  Serial.println(" MB");
  
  // ЗАЧЕМ: Функция ESP.getPsramSize() возвращает размер PSRAM.
  // Если PSRAM не обнаружена или настроена неправильно, вернётся 0.
  uint32_t psramSize = ESP.getPsramSize() / (1024 * 1024);
  Serial.print("PSRAM память: ");
  Serial.print(psramSize);
  Serial.println(" MB");
  
  // ЗАЧЕМ: Проверяем, действительно ли PSRAM доступна для использования.
  if (psramSize > 0) {
    Serial.println("✅ PSRAM работает! Можно использовать для больших буферов.");
  } else {
    Serial.println("❌ PSRAM НЕ обнаружена. Проверь настройки Tools -> PSRAM -> OPI PSRAM");
  }
  
  // ЗАЧЕМ: Показываем, сколько оперативной памяти (SRAM) свободно прямо сейчас.
  Serial.print("Свободно обычной SRAM: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" байт");
}

void loop() {
  // Этот тест нужен только один раз при старте, поэтому loop пустой.
  delay(10000);
}