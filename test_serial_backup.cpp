#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    delay(1000); // Aguardar estabilização
    Serial.println("=== ESP32-C3 Teste Serial ===");
    Serial.println("Sistema iniciado com sucesso!");
    Serial.printf("Chip ID: %llX\n", ESP.getEfuseMac());
    Serial.printf("Flash Size: %d bytes\n", ESP.getFlashChipSize());
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
}

void loop() {
    static unsigned long lastPrint = 0;
    static int counter = 0;
    
    if (millis() - lastPrint > 1000) {
        Serial.printf("Teste Serial #%d - Uptime: %lu ms\n", counter++, millis());
        lastPrint = millis();
    }
    
    delay(100);
}