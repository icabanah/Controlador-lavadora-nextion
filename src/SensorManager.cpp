#include "SensorManager.h"

SensorManager::SensorManager()
    : monitoringActive(false),
      oneWire(HardwarePins::TEMPERATURE),
      tempSensor(&oneWire),
      tempSensorFound(false),
      currentTemperature(0.0),
      temperatureValid(false),
      lastTempRead(0),
      tempConversionInProgress(false),
      currentPressure(0),
      currentWaterLevel(0),
      lastPressureRead(0) {}

// ========================================
// Inicialización
// ========================================

void SensorManager::begin() {
    // Inicializar sensor de temperatura
    tempSensor.begin();

    // IMPORTANTE: Configurar para lecturas asíncronas (no bloqueantes)
    tempSensor.setWaitForConversion(false);

    // Verificar si hay CUALQUIER dispositivo en el bus OneWire.
    // oneWire.search() no se bloquea si no hay nada conectado.
    uint8_t addr[8];
    if (oneWire.search(addr)) {
        tempSensorFound = true;
        tempSensor.setResolution(SensorConfig::TEMP_RESOLUTION);
        Serial.println("Sensor de temperatura DS18B20 encontrado.");
        // Opcional: verificar si la dirección encontrada es la que esperamos
        // if (memcmp(addr, SensorConfig::TEMP_SENSOR_ADDR, 8) != 0) {
        //     Serial.println("ADVERTENCIA: La dirección del sensor no coincide con la configurada.");
        // }
    } else {
        Serial.println("ADVERTENCIA: Sensor de temperatura DS18B20 NO encontrado. Funcionando sin él.");
    }

    // Inicializar sensor de presión
    pressureSensor.begin(HardwarePins::PRESSURE_DOUT, HardwarePins::PRESSURE_SCLK);

    // NO usar tare() para evitar bloqueos en el inicio
    // La calibración se hará manualmente ajustando PRESSURE_OFFSET en Config.h
    Serial.println("Sensor de presión inicializado (sin auto-calibración).");

    // Mostrar umbrales de calibración
    Serial.println("\n=== Umbrales de presión configurados ===");
    Serial.printf("Nivel 0: < %d (sin agua)\n", SensorConfig::PRESSURE_LEVEL_1);
    Serial.printf("Nivel 1: %d - %d\n", SensorConfig::PRESSURE_LEVEL_1, SensorConfig::PRESSURE_LEVEL_2 - 1);
    Serial.printf("Nivel 2: %d - %d\n", SensorConfig::PRESSURE_LEVEL_2, SensorConfig::PRESSURE_LEVEL_3 - 1);
    Serial.printf("Nivel 3: %d - %d\n", SensorConfig::PRESSURE_LEVEL_3, SensorConfig::PRESSURE_LEVEL_4 - 1);
    Serial.printf("Nivel 4: >= %d (lleno)\n", SensorConfig::PRESSURE_LEVEL_4);
    Serial.println("========================================\n");

    // Primera lectura
    forceRead();
}

// ========================================
// Actualización periódica
// ========================================

void SensorManager::update() {
    // Solo actualizar si el monitoreo está activo
    if (!monitoringActive) {
        return;
    }

    // Temperatura: Llamar SIEMPRE (es asíncrona, no bloquea)
    // Esto permite verificar constantemente si la conversión terminó
    readTemperature();

    // Presión: También asíncrona, verifica intervalo internamente
    readPressure();
}

// ========================================
// Control de monitoreo
// ========================================

void SensorManager::startMonitoring() {
    monitoringActive = true;
    Serial.println("[SENSOR] Monitoreo ACTIVADO");
}

void SensorManager::stopMonitoring() {
    monitoringActive = false;
    tempConversionInProgress = false;  // Cancelar conversión en progreso
    Serial.println("[SENSOR] Monitoreo DESACTIVADO");
}

// ========================================
// Lectura forzada inmediata
// ========================================

void SensorManager::forceRead() {
    readTemperature();
    readPressure();
}

// ========================================
// Verificaciones de estado
// ========================================

bool SensorManager::hasReachedLevel(uint8_t targetLevel) const {
    return currentWaterLevel >= targetLevel;
}

bool SensorManager::hasReachedTemperature(uint8_t targetTemp, uint8_t tolerance) const {
    if (!temperatureValid) return false;
    return abs(currentTemperature - targetTemp) <= tolerance;
}

bool SensorManager::isTemperatureTooHigh(uint8_t targetTemp, uint8_t tolerance) const {
    if (!temperatureValid) return false;
    return currentTemperature > (targetTemp + tolerance);
}

bool SensorManager::isTemperatureTooLow(uint8_t targetTemp, uint8_t tolerance) const {
    if (!temperatureValid) return false;
    return currentTemperature < (targetTemp - tolerance);
}

// ========================================
// Métodos privados
// ========================================

void SensorManager::readTemperature() {
    // No hacer nada si el sensor no fue encontrado en el arranque
    if (!tempSensorFound) {
        temperatureValid = false;
        return;
    }

    // Lectura ASÍNCRONA (no bloqueante) - igual que código anterior
    if (tempConversionInProgress) {
        // Verificar si la conversión terminó
        if (tempSensor.isConversionComplete()) {
            float temp = tempSensor.getTempC(SensorConfig::TEMP_SENSOR_ADDR);
            tempConversionInProgress = false;
            lastTempRead = millis();  // Actualizar timestamp al completar

            if (temp != DEVICE_DISCONNECTED_C && temp >= -55 && temp <= 125) {
                currentTemperature = temp;
                temperatureValid = true;
            } else {
                temperatureValid = false;
                Serial.println("Error: Sensor de temperatura desconectado");
            }
        }
        // Si no terminó, esperar al próximo ciclo (NO BLOQUEA)
    } else {
        // Solo iniciar nueva conversión si pasó el intervalo
        unsigned long now = millis();
        if (now - lastTempRead >= Timing::SENSOR_READ_INTERVAL_MS) {
            tempSensor.requestTemperatures();
            tempConversionInProgress = true;
        }
    }
}

void SensorManager::readPressure() {
    // OPTIMIZACIÓN: Verificar is_ready() es muy rápido, pero leer puede tardar
    // Solo leer si realmente pasó el intervalo completo desde última lectura
    unsigned long now = millis();
    if (now - lastPressureRead < Timing::SENSOR_READ_INTERVAL_MS) {
        return;  // Salir rápido si no es momento de leer
    }

    if (pressureSensor.is_ready()) {
        // Leer solo con pascal() (como código anterior)
        float pressurePascal = pressureSensor.pascal();
        currentPressure = (long)pressurePascal;
        currentWaterLevel = calculateWaterLevel(currentPressure);
        lastPressureRead = now;  // Actualizar timestamp aquí

        // Debug deshabilitado (ralentiza el sistema)
        // Serial.printf("[SENSOR] Presion: %.2f Pa → Nivel: %d\n",
        //               pressurePascal, currentWaterLevel);
    }
}

uint8_t SensorManager::calculateWaterLevel(long pressure) {
    // Aplicar offset manual (sin tare)
    float pressureFloat = (float)(pressure - SensorConfig::PRESSURE_OFFSET);

    if (pressureFloat < SensorConfig::PRESSURE_LEVEL_1) {
        return 0;  // Sin agua
    }

    if (pressureFloat < SensorConfig::PRESSURE_LEVEL_2) {
        float p = (pressureFloat - SensorConfig::PRESSURE_LEVEL_1) /
                  (SensorConfig::PRESSURE_LEVEL_2 - SensorConfig::PRESSURE_LEVEL_1);
        uint8_t level = (uint8_t)(1 + p);
        return (level < 1) ? 1 : ((level > 2) ? 2 : level);  // Nivel 1-2 interpolado
    }

    if (pressureFloat < SensorConfig::PRESSURE_LEVEL_3) {
        float p = (pressureFloat - SensorConfig::PRESSURE_LEVEL_2) /
                  (SensorConfig::PRESSURE_LEVEL_3 - SensorConfig::PRESSURE_LEVEL_2);
        uint8_t level = (uint8_t)(2 + p);
        return (level < 2) ? 2 : ((level > 3) ? 3 : level);  // Nivel 2-3 interpolado
    }

    if (pressureFloat < SensorConfig::PRESSURE_LEVEL_4) {
        float p = (pressureFloat - SensorConfig::PRESSURE_LEVEL_3) /
                  (SensorConfig::PRESSURE_LEVEL_4 - SensorConfig::PRESSURE_LEVEL_3);
        uint8_t level = (uint8_t)(3 + p);
        return (level < 3) ? 3 : ((level > 4) ? 4 : level);  // Nivel 3-4 interpolado
    }

    return 4;  // Máximo nivel
}
