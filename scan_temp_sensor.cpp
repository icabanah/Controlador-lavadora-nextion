// Programa temporal para escanear dirección del sensor DS18B20
// Compilar y cargar para obtener la dirección del nuevo sensor
// ESCANEA CONTINUAMENTE cada 3 segundos

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 23  // GPIO 23 según Config.h

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

unsigned long lastScan = 0;
const unsigned long SCAN_INTERVAL = 3000;  // Escanear cada 3 segundos

void scanSensors() {
    Serial.println("\n========================================");
    Serial.println("Escaneando sensores DS18B20...");
    Serial.print("Tiempo: ");
    Serial.print(millis() / 1000);
    Serial.println(" segundos desde inicio");
    Serial.println("========================================\n");

    // Obtener número de dispositivos
    uint8_t deviceCount = sensors.getDeviceCount();
    Serial.print("Dispositivos encontrados: ");
    Serial.println(deviceCount);
    Serial.println();

    if (deviceCount == 0) {
        Serial.println("ERROR: No se encontraron sensores.");
        Serial.println("Verifica las conexiones:");
        Serial.println("  - VCC: 3.3V o 5V");
        Serial.println("  - GND: GND");
        Serial.println("  - DATA: GPIO 23");
        Serial.println("  - Resistor pull-up: 4.7kΩ entre DATA y VCC");
        Serial.println("\nReescaneando en 3 segundos...\n");
        return;
    }

    // Escanear cada dispositivo
    uint8_t addr[8];
    oneWire.reset_search();

    for (uint8_t i = 0; i < deviceCount; i++) {
        Serial.print("Sensor #");
        Serial.print(i + 1);
        Serial.println(":");

        if (oneWire.search(addr)) {
            // Mostrar dirección en formato de array para Config.h
            Serial.print("  Dirección: {");
            for (int j = 0; j < 8; j++) {
                Serial.print("0x");
                if (addr[j] < 16) Serial.print("0");
                Serial.print(addr[j], HEX);
                if (j < 7) Serial.print(", ");
            }
            Serial.println("}");

            // Verificar CRC
            if (OneWire::crc8(addr, 7) != addr[7]) {
                Serial.println("  WARNING: CRC inválido!");
            } else {
                Serial.println("  CRC: OK");
            }

            // Leer temperatura
            sensors.requestTemperatures();
            float tempC = sensors.getTempC(addr);

            if (tempC != DEVICE_DISCONNECTED_C) {
                Serial.print("  Temperatura actual: ");
                Serial.print(tempC);
                Serial.println(" °C");
            } else {
                Serial.println("  ERROR: No se pudo leer temperatura");
            }

            Serial.println();
        } else {
            Serial.println("  ERROR: No se pudo leer dirección");
        }
    }

    Serial.println("========================================");
    Serial.println("*** COPIA LA DIRECCIÓN DE ARRIBA ***");
    Serial.println("Reescaneando en 3 segundos...");
    Serial.println("========================================");
}

void setup() {
    Serial.begin(115200);

    // Esperar 5 segundos para que se abra el monitor serial
    for (int i = 5; i > 0; i--) {
        Serial.print("Iniciando escaneo en ");
        Serial.print(i);
        Serial.println(" segundos...");
        delay(1000);
    }

    sensors.begin();

    // Primera escaneo inmediato
    scanSensors();
    lastScan = millis();
}

void loop() {
    // Escanear cada 3 segundos
    if (millis() - lastScan >= SCAN_INTERVAL) {
        scanSensors();
        lastScan = millis();
    }
}
