#include "Storage.h"

// Namespace para almacenamiento
namespace StorageConfig {
    constexpr const char* NAMESPACE = "washer";  // Namespace de Preferences
    constexpr const char* KEY_INITIALIZED = "init";
}

Storage::Storage() {
}

void Storage::begin() {
    preferences.begin(StorageConfig::NAMESPACE, false);  // false = read/write

    // Verificar si es la primera vez que se inicia
    bool initialized = preferences.getBool(StorageConfig::KEY_INITIALIZED, false);

    if (!initialized) {
        Serial.println("[STORAGE] Primera inicialización - Restaurando valores de fábrica");
        restoreDefaults();
        preferences.putBool(StorageConfig::KEY_INITIALIZED, true);
    } else {
        Serial.println("[STORAGE] Configuraciones encontradas en memoria");
    }

    preferences.end();
}

String Storage::getKey(uint8_t programNumber, const char* param, uint8_t processIndex) {
    char key[16];
    snprintf(key, sizeof(key), "p%d_%d_%s", programNumber, processIndex, param);
    return String(key);
}

bool Storage::saveProgram(uint8_t programNumber, const ProgramConfig& config) {
    preferences.begin(StorageConfig::NAMESPACE, false);

    Serial.printf("[STORAGE] Guardando configuración del programa %d\n", programNumber);

    // Guardar número de procesos totales
    String key = getKey(programNumber, "total");
    preferences.putUChar(key.c_str(), config.totalProcesses);

    // Guardar configuración de cada proceso
    for (uint8_t i = 0; i < config.totalProcesses; i++) {
        // Nivel de agua
        key = getKey(programNumber, "nivel", i);
        preferences.putUChar(key.c_str(), config.waterLevel[i]);

        // Temperatura
        key = getKey(programNumber, "temp", i);
        preferences.putUChar(key.c_str(), config.temperature[i]);

        // Tiempo
        key = getKey(programNumber, "time", i);
        preferences.putUChar(key.c_str(), config.time[i]);

        // Centrifugado
        key = getKey(programNumber, "centrif", i);
        preferences.putBool(key.c_str(), config.centrifugeEnabled[i]);

        // Tipo de agua
        key = getKey(programNumber, "water", i);
        preferences.putUChar(key.c_str(), config.waterType[i]);
    }

    preferences.end();
    Serial.printf("[STORAGE] Programa %d guardado exitosamente\n", programNumber);
    return true;
}

bool Storage::loadProgram(uint8_t programNumber, ProgramConfig& config) {
    preferences.begin(StorageConfig::NAMESPACE, true);  // true = read-only

    // Verificar si existe configuración para este programa
    String key = getKey(programNumber, "total");
    if (!preferences.isKey(key.c_str())) {
        Serial.printf("[STORAGE] No hay configuración guardada para programa %d\n", programNumber);
        preferences.end();
        return false;
    }

    Serial.printf("[STORAGE] Cargando configuración del programa %d\n", programNumber);

    // Cargar número de procesos
    config.totalProcesses = preferences.getUChar(key.c_str(), 1);

    // Cargar configuración de cada proceso
    for (uint8_t i = 0; i < config.totalProcesses; i++) {
        // Nivel de agua
        key = getKey(programNumber, "nivel", i);
        config.waterLevel[i] = preferences.getUChar(key.c_str(), Limits::MAX_WATER_LEVEL);

        // Temperatura
        key = getKey(programNumber, "temp", i);
        config.temperature[i] = preferences.getUChar(key.c_str(), 60);

        // Tiempo
        key = getKey(programNumber, "time", i);
        config.time[i] = preferences.getUChar(key.c_str(), 15);

        // Centrifugado
        key = getKey(programNumber, "centrif", i);
        config.centrifugeEnabled[i] = preferences.getBool(key.c_str(), true);

        // Tipo de agua
        key = getKey(programNumber, "water", i);
        config.waterType[i] = (WaterType)preferences.getUChar(key.c_str(), WATER_HOT);
    }

    preferences.end();
    Serial.printf("[STORAGE] Programa %d cargado exitosamente\n", programNumber);
    return true;
}

bool Storage::saveProcess(uint8_t programNumber, uint8_t processIndex, const ProgramConfig& config) {
    preferences.begin(StorageConfig::NAMESPACE, false);

    Serial.printf("[STORAGE] Guardando proceso %d del programa %d\n", processIndex + 1, programNumber);

    String key;

    // Nivel de agua
    key = getKey(programNumber, "nivel", processIndex);
    preferences.putUChar(key.c_str(), config.waterLevel[processIndex]);

    // Temperatura
    key = getKey(programNumber, "temp", processIndex);
    preferences.putUChar(key.c_str(), config.temperature[processIndex]);

    // Tiempo
    key = getKey(programNumber, "time", processIndex);
    preferences.putUChar(key.c_str(), config.time[processIndex]);

    // Centrifugado
    key = getKey(programNumber, "centrif", processIndex);
    preferences.putBool(key.c_str(), config.centrifugeEnabled[processIndex]);

    // Tipo de agua
    key = getKey(programNumber, "water", processIndex);
    preferences.putUChar(key.c_str(), config.waterType[processIndex]);

    preferences.end();
    Serial.printf("[STORAGE] Proceso %d guardado exitosamente\n", processIndex + 1);
    return true;
}

void Storage::restoreDefaults() {
    Serial.println("[STORAGE] Restaurando valores de fábrica...");

    preferences.begin(StorageConfig::NAMESPACE, false);

    // Limpiar todas las claves
    preferences.clear();

    preferences.end();

    // Crear configuraciones por defecto para cada programa
    ProgramConfig config;

    // Programa 22 - Agua Caliente
    config.setDefaults(PROGRAM_22);
    saveProgram(22, config);

    // Programa 23 - Agua Fría
    config.setDefaults(PROGRAM_23);
    saveProgram(23, config);

    // Programa 24 - Multiproceso
    config.setDefaults(PROGRAM_24);
    saveProgram(24, config);

    Serial.println("[STORAGE] Valores de fábrica restaurados");
}

bool Storage::hasStoredConfig(uint8_t programNumber) {
    preferences.begin(StorageConfig::NAMESPACE, true);
    String key = getKey(programNumber, "total");
    bool exists = preferences.isKey(key.c_str());
    preferences.end();
    return exists;
}

void Storage::clearAll() {
    Serial.println("[STORAGE] Limpiando toda la memoria...");
    preferences.begin(StorageConfig::NAMESPACE, false);
    preferences.clear();
    preferences.putBool(StorageConfig::KEY_INITIALIZED, false);
    preferences.end();
    Serial.println("[STORAGE] Memoria limpiada");
}
