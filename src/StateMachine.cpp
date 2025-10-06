#include "StateMachine.h"
#include "HardwareControl.h"
#include "SensorManager.h"

// Variables externas (definidas en main.cpp)
extern HardwareControl hardware;
extern SensorManager sensors;

// ========================================
// ProgramConfig - Configuración por defecto
// ========================================

void ProgramConfig::setDefaults(uint8_t program) {
    programNumber = program;
    currentProcess = 0;
    currentPhase = PHASE_FILLING;

    if (program == PROGRAM_22) {
        // Programa 22: Agua caliente
        totalProcesses = 1;
        waterLevel[0] = 3;
        temperature[0] = 25;
        time[0] = 15;
        centrifugeEnabled[0] = false;
        waterType[0] = WATER_HOT;

    } else if (program == PROGRAM_23) {
        // Programa 23: Agua fría
        totalProcesses = 1;
        waterLevel[0] = 3;
        temperature[0] = 25;
        time[0] = 15;
        centrifugeEnabled[0] = false;
        waterType[0] = WATER_COLD;

    } else if (program == PROGRAM_24) {
        // Programa 24: Multiproceso
        totalProcesses = 4;
        for (int i = 0; i < 4; i++) {
            waterLevel[i] = 3;
            temperature[i] = 25;
            time[i] = 15;
            centrifugeEnabled[i] = false;
            waterType[i] = WATER_COLD;
        }
    }
}

// ========================================
// Constructor
// ========================================

StateMachine::StateMachine()
    : currentState(STATE_INIT),
      previousState(STATE_INIT),
      stateStartTime(0),
      phaseStartTime(0),
      programStartTime(0),
      pauseStartTime(0),
      totalPausedTime(0),
      pausedPhaseElapsedTime(0) {}

// ========================================
// Inicialización
// ========================================

void StateMachine::begin() {
    // Seleccionar programa por defecto (P22)
    config.setDefaults(PROGRAM_22);
    setState(STATE_INIT);
}

// ========================================
// Actualización principal
// ========================================

void StateMachine::update() {
    switch (currentState) {
        case STATE_INIT:        updateInit();       break;
        case STATE_WELCOME:     updateWelcome();    break;
        case STATE_SELECTION:   updateSelection();  break;
        case STATE_FILLING:     updateFilling();    break;
        case STATE_WASHING:     updateWashing();    break;
        case STATE_DRAINING:    updateDraining();   break;
        case STATE_SPINNING:    updateSpinning();   break;
        case STATE_RESTING:     updateResting();    break;
        case STATE_COOLING:     updateCooling();    break;
        case STATE_PAUSED:      updatePaused();     break;
        case STATE_COMPLETED:   updateCompleted();  break;
        case STATE_ERROR:       updateError();      break;
        case STATE_EMERGENCY:   updateEmergency();  break;
    }
}

// ========================================
// Control de estados
// ========================================

void StateMachine::setState(SystemState newState) {
    previousState = currentState;
    currentState = newState;
    stateStartTime = millis();

    Serial.print("Estado: ");
    Serial.println(currentState);
}

// ========================================
// Control de programa
// ========================================

void StateMachine::selectProgram(uint8_t programNum) {
    config.setDefaults(programNum);
}

void StateMachine::startProgram() {
    if (currentState != STATE_SELECTION) return;

    // Inicializar contadores
    config.currentProcess = 0;
    config.currentPhase = PHASE_FILLING;
    programStartTime = millis();
    totalPausedTime = 0;

    // Cerrar drenaje y bloquear puerta
    hardware.closeDrain();
    hardware.lockDoor();

    setState(STATE_FILLING);
}

void StateMachine::pauseProgram() {
    if (currentState >= STATE_FILLING && currentState <= STATE_COOLING) {
        pauseStartTime = millis();

        // Guardar tiempo transcurrido de la fase actual
        pausedPhaseElapsedTime = millis() - phaseStartTime;

        // Guardar estado actual ANTES de cambiar a PAUSED
        previousState = currentState;
        currentState = STATE_PAUSED;
        stateStartTime = millis();

        // Detener completamente el programa
        hardware.stopMotor();           // Detener motor (izquierda/derecha)
        hardware.stopCentrifuge();      // Detener centrifugado
        hardware.closeWaterValves();    // Cerrar válvulas de llenado
        hardware.closeDrain();          // Cerrar drenaje
        // Nota: La puerta permanece cerrada (bloqueada)
    }
}

void StateMachine::resumeProgram() {
    if (currentState == STATE_PAUSED) {
        totalPausedTime += millis() - pauseStartTime;

        // Restaurar estado anterior
        currentState = previousState;
        stateStartTime = millis();

        // Restaurar phaseStartTime ajustando por el tiempo ya transcurrido
        // phaseStartTime debe ser "ahora - tiempo_transcurrido"
        phaseStartTime = millis() - pausedPhaseElapsedTime;

        // Resetear tiempo pausado de fase
        pausedPhaseElapsedTime = 0;
    }
}

void StateMachine::stopProgram() {
    hardware.stopMotor();
    hardware.stopCentrifuge();
    hardware.closeWaterValves();
    hardware.openDrain();  // ABRIR drenaje al detener programa (para vaciar agua)
    hardware.unlockDoor();  // Abrir puerta al detener programa
    setState(STATE_SELECTION);
}

void StateMachine::emergencyStop() {
    hardware.emergencyShutdown();
    setState(STATE_EMERGENCY);
}

// ========================================
// Actualización por estado
// ========================================

void StateMachine::updateInit() {
    setState(STATE_WELCOME);
}

void StateMachine::updateWelcome() {
    if (millis() - stateStartTime >= Timing::WELCOME_SCREEN_MS) {
        setState(STATE_SELECTION);
    }
}

void StateMachine::updateSelection() {
    // Esperando selección del usuario (manejado por callbacks de Nextion)
}

void StateMachine::updateFilling() {
    uint8_t proc = config.currentProcess;
    uint8_t targetLevel = config.waterLevel[proc];
    WaterType waterType = config.waterType[proc];

    // Abrir válvula apropiada
    if (waterType == WATER_HOT) {
        hardware.openHotWater();
    } else {
        hardware.openColdWater();
    }

    // Verificar si se alcanzó el nivel
    if (sensors.hasReachedLevel(targetLevel)) {
        hardware.closeWaterValves();
        nextPhase();
    }
}

void StateMachine::updateWashing() {
    uint8_t proc = config.currentProcess;

    // Alternar dirección del motor automáticamente
    hardware.toggleMotorDirection();

    // Temperatura es solo informativa (no hay control activo)
    // P22: agua caliente, P23: agua fría, P24: configurable por proceso

    // Verificar tiempo de lavado
    if (getPhaseElapsedTime() >= config.time[proc] * 60000UL) {
        hardware.stopMotor();
        hardware.closeWaterValves();
        nextPhase();
    }
}

void StateMachine::updateDraining() {
    hardware.openDrain();

    if (millis() - phaseStartTime >= Timing::DRAIN_TIME_SEC * 1000UL) {
        nextPhase();
    }
}

void StateMachine::updateSpinning() {
    uint8_t proc = config.currentProcess;

    if (config.centrifugeEnabled[proc]) {
        hardware.startCentrifuge();
        hardware.openDrain();  // Drenaje abierto durante centrifugado

        if (millis() - phaseStartTime >= Timing::CENTRIFUGE_TIME_SEC * 1000UL) {
            hardware.stopCentrifuge();

            // Verificar si es el último proceso
            if (isLastProcess()) {
                // Último proceso: ir a enfriamiento
                nextPhase();  // PHASE_COOLING
            } else {
                // No es el último: ir a reposo entre tandas (solo P24)
                phaseStartTime = millis();
                setState(STATE_RESTING);
            }
        }
    } else {
        // Saltar centrifugado
        // Verificar si es el último proceso
        if (isLastProcess()) {
            nextPhase();  // PHASE_COOLING
        } else {
            // No es el último: ir a reposo entre tandas (solo P24)
            phaseStartTime = millis();
            setState(STATE_RESTING);
        }
    }
}

void StateMachine::updateResting() {
    // Tiempo de reposo entre tandas (solo P24)
    // Permite que el agua drene completamente y los motores se detengan por inercia

    if (millis() - phaseStartTime >= Timing::REST_BETWEEN_PROCESS_SEC * 1000UL) {
        // Reposo completado, ir al siguiente proceso
        nextProcess();
    }
}

void StateMachine::updateCooling() {
    // La puerta se abre al inicio de la fase de enfriamiento
    // (demora 1 minuto en abrirse naturalmente, coincide con el tiempo de enfriamiento)

    if (millis() - phaseStartTime >= Timing::COOLING_TIME_SEC * 1000UL) {
        // El enfriamiento SOLO ocurre al final de todos los procesos
        setState(STATE_COMPLETED);
    }
}

void StateMachine::updatePaused() {
    // Esperando que el usuario reanude o detenga
}

void StateMachine::updateCompleted() {
    // Programa completado, esperando nueva selección
}

void StateMachine::updateError() {
    hardware.emergencyShutdown();
}

void StateMachine::updateEmergency() {
    // Sistema detenido, requiere reinicio manual
}

// ========================================
// Transiciones
// ========================================

void StateMachine::nextPhase() {
    config.currentPhase++;
    phaseStartTime = millis();

    switch (config.currentPhase) {
        case PHASE_WASHING:
            setState(STATE_WASHING);
            break;
        case PHASE_DRAINING:
            setState(STATE_DRAINING);
            break;
        case PHASE_SPINNING:
            setState(STATE_SPINNING);
            break;
        case PHASE_COOLING:
            // Abrir puerta al inicio de la fase de enfriamiento
            hardware.unlockDoor();
            setState(STATE_COOLING);
            break;
        default:
            setState(STATE_ERROR);
    }
}

void StateMachine::nextProcess() {
    config.currentProcess++;
    config.currentPhase = PHASE_FILLING;
    phaseStartTime = millis();

    hardware.closeDrain();  // Cerrar drenaje para el siguiente proceso
    setState(STATE_FILLING);
}

bool StateMachine::isLastPhase() const {
    return config.currentPhase == PHASE_COOLING;
}

bool StateMachine::isLastProcess() const {
    return config.currentProcess >= (config.totalProcesses - 1);
}

// ========================================
// Información de tiempo
// ========================================

unsigned long StateMachine::getPhaseElapsedTime() const {
    // Si está pausado, devolver el tiempo guardado al pausar
    if (currentState == STATE_PAUSED) {
        return pausedPhaseElapsedTime;
    }
    return millis() - phaseStartTime;
}

unsigned long StateMachine::getPhaseRemainingTime() const {
    unsigned long elapsed = getPhaseElapsedTime();
    unsigned long targetTime = 0;

    // Si está pausado, usar el estado anterior para determinar el targetTime
    SystemState stateToCheck = (currentState == STATE_PAUSED) ? previousState : currentState;

    switch (stateToCheck) {
        case STATE_WASHING:
            // Tiempo de lavado configurado (en minutos)
            targetTime = config.time[config.currentProcess] * 60000UL;
            break;

        case STATE_DRAINING:
            // Tiempo de drenaje fijo
            targetTime = Timing::DRAIN_TIME_SEC * 1000UL;
            break;

        case STATE_SPINNING:
            // Tiempo de centrifugado fijo (solo si está habilitado)
            if (config.centrifugeEnabled[config.currentProcess]) {
                targetTime = Timing::CENTRIFUGE_TIME_SEC * 1000UL;
            }
            break;

        case STATE_RESTING:
            // Tiempo de reposo entre tandas
            targetTime = Timing::REST_BETWEEN_PROCESS_SEC * 1000UL;
            break;

        case STATE_COOLING:
            // Tiempo de enfriamiento fijo
            targetTime = Timing::COOLING_TIME_SEC * 1000UL;
            break;

        case STATE_FILLING:
        default:
            // Fase de llenado no tiene tiempo definido (depende del sensor)
            return 0;
    }

    if (elapsed >= targetTime) {
        return 0;
    }

    return targetTime - elapsed;
}

bool StateMachine::isTimerActive() const {
    // El timer está activo en todas las fases excepto llenado
    return currentState == STATE_WASHING ||
           currentState == STATE_DRAINING ||
           currentState == STATE_SPINNING ||
           currentState == STATE_RESTING ||
           currentState == STATE_COOLING;
}

uint16_t StateMachine::getTotalProgramTime() const {
    // Tiempo total = suma de todos los tiempos de lavado configurados
    uint16_t totalSeconds = 0;

    for (uint8_t i = 0; i < config.totalProcesses; i++) {
        // Solo tiempo de lavado (configurado en minutos)
        totalSeconds += config.time[i] * 60;
    }

    return totalSeconds;
}

unsigned long StateMachine::getTotalElapsedTime() const {
    return millis() - programStartTime - totalPausedTime;
}

void StateMachine::resetTimers() {
    programStartTime = millis();
    phaseStartTime = millis();
    totalPausedTime = 0;
}
