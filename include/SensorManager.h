#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <HX710B.h>
#include "Config.h"

class SensorManager {
public:
    SensorManager();

    void begin();
    void update();

    // Control de monitoreo (activar/desactivar sensores)
    void startMonitoring();
    void stopMonitoring();
    bool isMonitoring() const { return monitoringActive; }

    // Lectura de temperatura
    float getTemperature() const { return currentTemperature; }
    bool isTemperatureReady() const { return temperatureValid; }

    // Lectura de nivel de agua
    uint8_t getWaterLevel() const { return currentWaterLevel; }
    long getPressureRaw() const { return currentPressure; }

    // Verificaciones de estado
    bool hasReachedLevel(uint8_t targetLevel) const;
    bool hasReachedTemperature(uint8_t targetTemp, uint8_t tolerance = SensorConfig::TEMP_TOLERANCE) const;
    bool isTemperatureTooHigh(uint8_t targetTemp, uint8_t tolerance = SensorConfig::TEMP_TOLERANCE) const;
    bool isTemperatureTooLow(uint8_t targetTemp, uint8_t tolerance = SensorConfig::TEMP_TOLERANCE) const;

    // Forzar lectura inmediata
    void forceRead();

private:
    // Control de monitoreo
    bool monitoringActive;

    // Sensor de temperatura
    OneWire oneWire;
    DallasTemperature tempSensor;
    float currentTemperature;
    bool temperatureValid;
    unsigned long lastTempRead;
    bool tempSensorFound;
    bool tempConversionInProgress;  // Flag para lectura asíncrona

    // Sensor de presión/nivel
    HX710B pressureSensor;
    long currentPressure;
    uint8_t currentWaterLevel;
    unsigned long lastPressureRead;

    // Métodos privados
    void readTemperature();
    void readPressure();
    uint8_t calculateWaterLevel(long pressure);
};

#endif // SENSOR_MANAGER_H
