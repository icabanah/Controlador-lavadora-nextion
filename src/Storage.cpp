#include "Storage.h"

namespace StorageConfig {
    constexpr const char* NAMESPACE = "washer";
    constexpr const char* KEY_INITIALIZED = "init";
}

Storage::Storage() {
}

void Storage::begin() {
    preferences.begin(StorageConfig::NAMESPACE, false);
    bool initialized = preferences.getBool(StorageConfig::KEY_INITIALIZED, false);
    preferences.end();

    if (!initialized) {
        restoreDefaults();
        preferences.begin(StorageConfig::NAMESPACE, false);
        preferences.putBool(StorageConfig::KEY_INITIALIZED, true);
        preferences.end();
    }
}

String Storage::getKey(uint8_t programNumber, const char* param, uint8_t processIndex) {
    char key[16];
    snprintf(key, sizeof(key), "p%d_%d_%s", programNumber, processIndex, param);
    return String(key);
}

bool Storage::saveProgram(uint8_t programNumber, const ProgramConfig& config) {
    preferences.begin(StorageConfig::NAMESPACE, false);

    String key = getKey(programNumber, "total");
    preferences.putUChar(key.c_str(), config.totalProcesses);

    for (uint8_t i = 0; i < config.totalProcesses; i++) {
        key = getKey(programNumber, "nivel", i);
        preferences.putUChar(key.c_str(), config.waterLevel[i]);

        key = getKey(programNumber, "temp", i);
        preferences.putUChar(key.c_str(), config.temperature[i]);

        key = getKey(programNumber, "time", i);
        preferences.putUChar(key.c_str(), config.time[i]);

        key = getKey(programNumber, "centrif", i);
        preferences.putBool(key.c_str(), config.centrifugeEnabled[i]);

        key = getKey(programNumber, "water", i);
        preferences.putUChar(key.c_str(), config.waterType[i]);
    }

    preferences.end();
    return true;
}

bool Storage::loadProgram(uint8_t programNumber, ProgramConfig& config) {
    preferences.begin(StorageConfig::NAMESPACE, true);

    String key = getKey(programNumber, "total");
    if (!preferences.isKey(key.c_str())) {
        preferences.end();
        return false;
    }

    config.programNumber = programNumber;
    config.currentProcess = 0;
    config.currentPhase = PHASE_FILLING;
    config.totalProcesses = preferences.getUChar(key.c_str(), 1);

    for (uint8_t i = 0; i < config.totalProcesses; i++) {
        key = getKey(programNumber, "nivel", i);
        config.waterLevel[i] = preferences.getUChar(key.c_str(), Limits::MAX_WATER_LEVEL);

        key = getKey(programNumber, "temp", i);
        config.temperature[i] = preferences.getUChar(key.c_str(), 60);

        key = getKey(programNumber, "time", i);
        config.time[i] = preferences.getUChar(key.c_str(), 15);

        key = getKey(programNumber, "centrif", i);
        config.centrifugeEnabled[i] = preferences.getBool(key.c_str(), true);

        key = getKey(programNumber, "water", i);
        config.waterType[i] = (WaterType)preferences.getUChar(key.c_str(), WATER_HOT);
    }

    preferences.end();
    return true;
}

bool Storage::saveProcess(uint8_t programNumber, uint8_t processIndex, const ProgramConfig& config) {
    preferences.begin(StorageConfig::NAMESPACE, false);

    String key = getKey(programNumber, "nivel", processIndex);
    preferences.putUChar(key.c_str(), config.waterLevel[processIndex]);

    key = getKey(programNumber, "temp", processIndex);
    preferences.putUChar(key.c_str(), config.temperature[processIndex]);

    key = getKey(programNumber, "time", processIndex);
    preferences.putUChar(key.c_str(), config.time[processIndex]);

    key = getKey(programNumber, "centrif", processIndex);
    preferences.putBool(key.c_str(), config.centrifugeEnabled[processIndex]);

    key = getKey(programNumber, "water", processIndex);
    preferences.putUChar(key.c_str(), config.waterType[processIndex]);

    preferences.end();
    return true;
}

void Storage::restoreDefaults() {
    ProgramConfig config;

    config.setDefaults(PROGRAM_22);
    saveProgram(22, config);

    config.setDefaults(PROGRAM_23);
    saveProgram(23, config);

    config.setDefaults(PROGRAM_24);
    saveProgram(24, config);
}

bool Storage::hasStoredConfig(uint8_t programNumber) {
    preferences.begin(StorageConfig::NAMESPACE, true);
    String key = getKey(programNumber, "total");
    bool exists = preferences.isKey(key.c_str());
    preferences.end();
    return exists;
}

void Storage::clearAll() {
    preferences.begin(StorageConfig::NAMESPACE, false);
    preferences.clear();
    preferences.putBool(StorageConfig::KEY_INITIALIZED, false);
    preferences.end();
}

void Storage::debugPrintAll() {
    preferences.begin(StorageConfig::NAMESPACE, true);
    for (uint8_t prog : {22, 23, 24}) {
        String key = getKey(prog, "total");
        if (preferences.isKey(key.c_str())) {
            uint8_t total = preferences.getUChar(key.c_str());
            Serial.printf("[STORAGE] P%d: %d procesos\n", prog, total);
        }
    }
    preferences.end();
}
