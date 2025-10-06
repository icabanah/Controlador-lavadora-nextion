#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include <Preferences.h>
#include "Config.h"
#include "StateMachine.h"

// ========================================
// CLASE STORAGE - ALMACENAMIENTO PERSISTENTE
// ========================================

class Storage {
public:
    Storage();

    // Inicialización
    void begin();

    // Guardar configuración completa de un programa
    bool saveProgram(uint8_t programNumber, const ProgramConfig& config);

    // Cargar configuración de un programa
    bool loadProgram(uint8_t programNumber, ProgramConfig& config);

    // Guardar configuración de un proceso específico
    bool saveProcess(uint8_t programNumber, uint8_t processIndex, const ProgramConfig& config);

    // Restaurar valores de fábrica
    void restoreDefaults();

    // Verificar si existen configuraciones guardadas
    bool hasStoredConfig(uint8_t programNumber);

    // Limpiar toda la memoria
    void clearAll();

    // Debug: imprimir todas las claves guardadas
    void debugPrintAll();

private:
    Preferences preferences;

    // Helpers para generar claves únicas
    String getKey(uint8_t programNumber, const char* param, uint8_t processIndex = 0);
};

#endif // STORAGE_H
