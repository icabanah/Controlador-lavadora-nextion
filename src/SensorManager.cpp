#include "SensorManager.h"

SensorManager::SensorManager()
    : oneWire(HardwarePins::TEMPERATURE),
      tempSensor(&oneWire),
      currentTemperature(0.0),
      temperatureValid(false),
      lastTempRead(0),
      currentPressure(0),
      currentWaterLevel(0),
      lastPressureRead(0) {}

// ========================================
// Inicialización
// ========================================

void SensorManager::begin() {
    // Inicializar sensor de temperatura
    tempSensor.begin();
    tempSensor.setResolution(SensorConfig::TEMP_RESOLUTION);

    // Inicializar sensor de presión
    pressureSensor.begin(HardwarePins::PRESSURE_DOUT, HardwarePins::PRESSURE_SCLK);

    // Primera lectura
    forceRead();
}

// ========================================
// Actualización periódica
// ========================================

void SensorManager::update() {
    unsigned long now = millis();

    // Leer temperatura cada intervalo
    if (now - lastTempRead >= Timing::SENSOR_READ_INTERVAL_MS) {
        readTemperature();
        lastTempRead = now;
    }

    // Leer presión cada intervalo
    if (now - lastPressureRead >= Timing::SENSOR_READ_INTERVAL_MS) {
        readPressure();
        lastPressureRead = now;
    }
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
    tempSensor.requestTemperatures();
    float temp = tempSensor.getTempC(SensorConfig::TEMP_SENSOR_ADDR);

    if (temp != DEVICE_DISCONNECTED_C && temp >= -55 && temp <= 125) {
        currentTemperature = temp;
        temperatureValid = true;
    } else {
        temperatureValid = false;
        Serial.println("Error: Sensor de temperatura desconectado");
    }
}

void SensorManager::readPressure() {
    if (pressureSensor.is_ready()) {
        long reading = pressureSensor.read();
        currentPressure = reading;
        currentWaterLevel = calculateWaterLevel(currentPressure);
    } else {
        Serial.println("Advertencia: Sensor de presión no listo");
    }
}

uint8_t SensorManager::calculateWaterLevel(long pressure) {
    // Determinar nivel según umbrales (ajustar estos valores según calibración)
    if (pressure < SensorConfig::PRESSURE_LEVEL_1) {
        return 0;  // Sin agua
    } else if (pressure < SensorConfig::PRESSURE_LEVEL_2) {
        return 1;
    } else if (pressure < SensorConfig::PRESSURE_LEVEL_3) {
        return 2;
    } else if (pressure < SensorConfig::PRESSURE_LEVEL_4) {
        return 3;
    } else {
        return 4;
    }
}
