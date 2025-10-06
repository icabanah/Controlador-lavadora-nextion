#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>
#include "Config.h"

// Estados del sistema
enum SystemState {
    STATE_INIT,
    STATE_WELCOME,
    STATE_SELECTION,
    STATE_FILLING,
    STATE_WASHING,
    STATE_DRAINING,
    STATE_SPINNING,
    STATE_COOLING,
    STATE_PAUSED,
    STATE_COMPLETED,
    STATE_ERROR,
    STATE_EMERGENCY
};

// Estructura para almacenar configuración del programa actual
struct ProgramConfig {
    uint8_t programNumber;      // 22, 23 o 24
    uint8_t currentProcess;     // 0-3 (para P24, sino siempre 0)
    uint8_t totalProcesses;     // 1 para P22/P23, 4 para P24
    uint8_t currentPhase;       // 0-4 (llenado, lavado, drenaje, centrifugado, enfriamiento)

    // Parámetros configurables
    uint8_t waterLevel[4];      // Nivel de agua por proceso
    uint8_t temperature[4];     // Temperatura objetivo por proceso
    uint8_t time[4];            // Tiempo de lavado por proceso (minutos)
    bool centrifugeEnabled[4];  // Centrifugado habilitado por proceso
    WaterType waterType[4];     // Tipo de agua por proceso

    // Valores por defecto
    void setDefaults(uint8_t program);
};

class StateMachine {
public:
    StateMachine();

    void begin();
    void update();

    // Control de estados
    void setState(SystemState newState);
    SystemState getState() const { return currentState; }

    // Control de programa
    void selectProgram(uint8_t programNum);
    void startProgram();
    void pauseProgram();
    void resumeProgram();
    void stopProgram();
    void emergencyStop();

    // Acceso a configuración
    ProgramConfig& getConfig() { return config; }

    // Info de tiempo
    unsigned long getPhaseElapsedTime() const;
    unsigned long getPhaseRemainingTime() const;  // Tiempo restante en fase de lavado (cuenta regresiva)
    unsigned long getTotalElapsedTime() const;
    bool isTimerActive() const;  // true si está en fase de lavado

private:
    SystemState currentState;
    SystemState previousState;
    ProgramConfig config;

    // Control de tiempo
    unsigned long stateStartTime;
    unsigned long phaseStartTime;
    unsigned long programStartTime;
    unsigned long pauseStartTime;
    unsigned long totalPausedTime;

    // Sub-estado para control de temperatura
    enum TempControlState {
        TEMP_IDLE,
        TEMP_DRAINING,
        TEMP_FILLING
    };
    TempControlState tempControlState;
    unsigned long tempControlStartTime;

    // Métodos de actualización por estado
    void updateInit();
    void updateWelcome();
    void updateSelection();
    void updateFilling();
    void updateWashing();
    void updateDraining();
    void updateSpinning();
    void updateCooling();
    void updatePaused();
    void updateCompleted();
    void updateError();
    void updateEmergency();

    // Transiciones de fase
    void nextPhase();
    void nextProcess();
    bool isLastPhase() const;
    bool isLastProcess() const;

    // Helpers
    void resetTimers();
    unsigned long getCurrentPhaseDuration() const;
};

#endif // STATE_MACHINE_H
