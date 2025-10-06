#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>
#include "esp_log.h"

// ========================================
// TAGS PARA LOGGING (por módulo)
// ========================================

static const char* TAG_MAIN = "MAIN";
static const char* TAG_STATE = "STATE";
static const char* TAG_HARDWARE = "HARDWARE";
static const char* TAG_SENSOR = "SENSOR";
static const char* TAG_NEXTION = "NEXTION";

// ========================================
// MACROS DE DEBUG USANDO esp_log
// ========================================

// Usar las funciones nativas de ESP32 (respetan CORE_DEBUG_LEVEL en platformio.ini)
// Niveles: log_v (Verbose=5), log_d (Debug=4), log_i (Info=3),
//          log_w (Warning=2), log_e (Error=1)

// Para estados
#define DEBUG_STATE_TRANSITION(from, to) \
    log_i(TAG_STATE, "Transición: %d -> %d", from, to)

// Para sensores
#define DEBUG_SENSOR_READ(name, value) \
    log_d(TAG_SENSOR, "%s = %.2f", name, value)

// Para hardware
#define DEBUG_HARDWARE_ACTION(action) \
    log_d(TAG_HARDWARE, "%s", action)

// Para eventos de Nextion
#define DEBUG_NEXTION_EVENT(page, comp, type) \
    log_d(TAG_NEXTION, "Event: Page=%d, Comp=%d, Type=%d", page, comp, type)

// Para errores
#define DEBUG_ERROR_MSG(tag, msg) \
    log_e(tag, "%s", msg)

// Para warnings
#define DEBUG_WARNING_MSG(tag, msg) \
    log_w(tag, "%s", msg)

// ========================================
// UTILIDADES DE DEBUG
// ========================================

namespace DebugUtils {

    // Configurar nivel de log en runtime (opcional)
    inline void setLogLevel(esp_log_level_t level) {
        esp_log_level_set("*", level);
    }

    // Niveles disponibles:
    // ESP_LOG_NONE    (0) - Sin logs
    // ESP_LOG_ERROR   (1) - Solo errores
    // ESP_LOG_WARN    (2) - Warnings y errores
    // ESP_LOG_INFO    (3) - Info, warnings y errores
    // ESP_LOG_DEBUG   (4) - Debug, info, warnings y errores
    // ESP_LOG_VERBOSE (5) - Todo

    // Mostrar estado de memoria
    inline void printMemoryInfo() {
        Serial.println("=== MEMORIA ===");
        Serial.printf("Heap libre: %d bytes\n", ESP.getFreeHeap());
        Serial.printf("Heap mínimo: %d bytes\n", ESP.getMinFreeHeap());
        Serial.printf("Tamaño heap: %d bytes\n", ESP.getHeapSize());
    }

    // Mostrar info del chip
    inline void printChipInfo() {
        Serial.println("=== INFO ESP32 ===");
        Serial.printf("Modelo: %s\n", ESP.getChipModel());
        Serial.printf("Revisión: %d\n", ESP.getChipRevision());
        Serial.printf("Cores: %d\n", ESP.getChipCores());
        Serial.printf("Frecuencia: %d MHz\n", ESP.getCpuFreqMHz());
        Serial.printf("Flash: %d MB\n", ESP.getFlashChipSize() / 1024 / 1024);
    }

    // Monitor de performance del loop
    class LoopMonitor {
    public:
        LoopMonitor() : lastUpdate(0), lastLoopTime(0), loopCount(0),
                        minLoopTime(999999), maxLoopTime(0) {}

        void start() {
            lastLoopTime = millis();
        }

        void update() {
            unsigned long now = millis();
            unsigned long loopTime = now - lastLoopTime;
            lastLoopTime = now;

            loopCount++;

            if (loopTime < minLoopTime) minLoopTime = loopTime;
            if (loopTime > maxLoopTime) maxLoopTime = loopTime;

            // Mostrar estadísticas cada 10 segundos
            if (now - lastUpdate >= 10000) {
                Serial.println("=== LOOP STATS ===");
                Serial.printf("Loops/seg: %lu\n", loopCount / 10);
                Serial.printf("Loop mín: %lu ms\n", minLoopTime);
                Serial.printf("Loop máx: %lu ms\n", maxLoopTime);

                lastUpdate = now;
                loopCount = 0;
                minLoopTime = 999999;
                maxLoopTime = 0;
            }
        }

    private:
        unsigned long lastUpdate;
        unsigned long lastLoopTime;
        unsigned long loopCount;
        unsigned long minLoopTime;
        unsigned long maxLoopTime;
    };

    // Simulador de sensores (para testing sin hardware)
    class SensorSimulator {
    public:
        static float getSimulatedTemperature() {
            static float temp = 25.0;
            temp += (random(-10, 11) / 20.0); // Variación gradual ±0.5°C
            temp = constrain(temp, 15.0, 35.0);
            return temp;
        }

        static uint8_t getSimulatedWaterLevel() {
            static uint8_t level = 0;
            static unsigned long lastChange = 0;

            // Simular llenado gradual cada 2 segundos
            if (millis() - lastChange > 2000 && level < 4) {
                level++;
                lastChange = millis();
            }
            return level;
        }

        static long getSimulatedPressure() {
            return 500 + (getSimulatedWaterLevel() * 40);
        }

        static void reset() {
            // Reiniciar simulación (implementar si es necesario)
        }
    };
}

#endif // DEBUG_H
