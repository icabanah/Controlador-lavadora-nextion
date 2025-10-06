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
      tempControlState(TEMP_IDLE),
      tempControlStartTime(0) {}

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
        setState(STATE_PAUSED);

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
        setState(previousState);
    }
}

void StateMachine::stopProgram() {
    hardware.resetAll();
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
    uint8_t targetTemp = config.temperature[proc];

    // Alternar dirección del motor automáticamente
    hardware.toggleMotorDirection();

    // Control de temperatura (solo para agua caliente) - Máquina de sub-estados
    if (config.waterType[proc] == WATER_HOT) {
        switch (tempControlState) {
            case TEMP_IDLE:
                // Verificar si necesita ajuste de temperatura
                if (sensors.isTemperatureTooHigh(targetTemp)) {
                    hardware.openDrain();
                    tempControlState = TEMP_DRAINING;
                    tempControlStartTime = millis();
                    Serial.println("Temp alta: drenando...");
                } else if (sensors.isTemperatureTooLow(targetTemp)) {
                    hardware.openDrain();
                    tempControlState = TEMP_DRAINING;
                    tempControlStartTime = millis();
                    Serial.println("Temp baja: drenando...");
                }
                break;

            case TEMP_DRAINING:
                // Esperar 5 segundos drenando
                if (millis() - tempControlStartTime >= 5000) {
                    hardware.closeDrain();

                    // Decidir qué agua agregar
                    if (sensors.isTemperatureTooHigh(targetTemp)) {
                        hardware.openColdWater();
                        Serial.println("Rellenando con agua fría...");
                    } else {
                        hardware.openHotWater();
                        Serial.println("Rellenando con agua caliente...");
                    }

                    tempControlState = TEMP_FILLING;
                    tempControlStartTime = millis();
                }
                break;

            case TEMP_FILLING:
                // Esperar 3 segundos rellenando
                if (millis() - tempControlStartTime >= 3000) {
                    hardware.closeWaterValves();
                    tempControlState = TEMP_IDLE;
                    Serial.println("Ajuste de temperatura completado");
                }
                break;
        }
    }

    // Verificar tiempo de lavado
    if (getPhaseElapsedTime() >= config.time[proc] * 60000UL) {
        hardware.stopMotor();
        hardware.closeWaterValves();
        tempControlState = TEMP_IDLE;  // Reset sub-estado
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
            nextPhase();
        }
    } else {
        // Saltar centrifugado
        nextPhase();
    }
}

void StateMachine::updateCooling() {
    if (millis() - phaseStartTime >= Timing::COOLING_TIME_SEC * 1000UL) {
        // Abrir puerta siempre al finalizar el enfriamiento
        hardware.unlockDoor();

        // Si es el último proceso, finalizar
        if (isLastProcess()) {
            setState(STATE_COMPLETED);
        } else {
            // Siguiente proceso (cerrar puerta nuevamente)
            hardware.lockDoor();
            nextProcess();
        }
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
    return millis() - phaseStartTime;
}

unsigned long StateMachine::getPhaseRemainingTime() const {
    // Solo en fase de lavado se cuenta el tiempo
    if (currentState != STATE_WASHING) {
        return 0;
    }

    uint8_t proc = config.currentProcess;
    unsigned long targetTime = config.time[proc] * 60000UL;  // Convertir minutos a ms
    unsigned long elapsed = getPhaseElapsedTime();

    if (elapsed >= targetTime) {
        return 0;
    }

    return targetTime - elapsed;
}

bool StateMachine::isTimerActive() const {
    return currentState == STATE_WASHING;
}

unsigned long StateMachine::getTotalElapsedTime() const {
    return millis() - programStartTime - totalPausedTime;
}

void StateMachine::resetTimers() {
    programStartTime = millis();
    phaseStartTime = millis();
    totalPausedTime = 0;
}
