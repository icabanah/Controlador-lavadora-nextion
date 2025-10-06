#include "HardwareControl.h"

HardwareControl::HardwareControl()
    : doorLocked(false),
      drainOpen(false),
      motorRunning(false),
      centrifugeActive(false),
      lastMotorToggle(0),
      motorState(MOTOR_RIGHT_ACTIVE) {}

// ========================================
// Inicialización
// ========================================

void HardwareControl::begin() {
    // Configurar pines de salida
    pinMode(HardwarePins::MOTOR_DIR_LEFT, OUTPUT);
    pinMode(HardwarePins::MOTOR_DIR_RIGHT, OUTPUT);
    pinMode(HardwarePins::CENTRIFUGE, OUTPUT);
    pinMode(HardwarePins::COLD_WATER_VALVE, OUTPUT);
    pinMode(HardwarePins::HOT_WATER_VALVE, OUTPUT);
    pinMode(HardwarePins::DRAIN_VALVE, OUTPUT);
    pinMode(HardwarePins::DOOR_MAGNET, OUTPUT);

    // Configurar entrada de emergencia
    pinMode(HardwarePins::EMERGENCY_BUTTON, INPUT_PULLUP);

    // Estado inicial seguro
    resetAll();
}

// ========================================
// Actualización periódica
// ========================================

void HardwareControl::update() {
    // Verificar emergencia automáticamente
    if (isEmergencyPressed()) {
        emergencyShutdown();
    }
}

// ========================================
// Control de válvulas
// ========================================

void HardwareControl::openColdWater() {
    digitalWrite(HardwarePins::COLD_WATER_VALVE, HIGH);
    closeWaterValves();  // Cerrar la otra antes
    digitalWrite(HardwarePins::COLD_WATER_VALVE, HIGH);
}

void HardwareControl::openHotWater() {
    digitalWrite(HardwarePins::HOT_WATER_VALVE, HIGH);
    closeWaterValves();  // Cerrar la otra antes
    digitalWrite(HardwarePins::HOT_WATER_VALVE, HIGH);
}

void HardwareControl::closeWaterValves() {
    digitalWrite(HardwarePins::COLD_WATER_VALVE, LOW);
    digitalWrite(HardwarePins::HOT_WATER_VALVE, LOW);
}

void HardwareControl::openDrain() {
    digitalWrite(HardwarePins::DRAIN_VALVE, HIGH);
    drainOpen = true;
}

void HardwareControl::closeDrain() {
    digitalWrite(HardwarePins::DRAIN_VALVE, LOW);
    drainOpen = false;
}

// ========================================
// Control de motor
// ========================================

void HardwareControl::startMotorLeft() {
    digitalWrite(HardwarePins::MOTOR_DIR_RIGHT, LOW);
    digitalWrite(HardwarePins::MOTOR_DIR_LEFT, HIGH);
    motorRunning = true;
    motorState = MOTOR_LEFT_ACTIVE;
    lastMotorToggle = millis();
    Serial.println("[HARDWARE] Motor IZQUIERDA activado");
}

void HardwareControl::startMotorRight() {
    digitalWrite(HardwarePins::MOTOR_DIR_LEFT, LOW);
    digitalWrite(HardwarePins::MOTOR_DIR_RIGHT, HIGH);
    motorRunning = true;
    motorState = MOTOR_RIGHT_ACTIVE;
    lastMotorToggle = millis();
    Serial.println("[HARDWARE] Motor DERECHA activado");
}

void HardwareControl::stopMotor() {
    digitalWrite(HardwarePins::MOTOR_DIR_LEFT, LOW);
    digitalWrite(HardwarePins::MOTOR_DIR_RIGHT, LOW);
    motorRunning = false;
}

void HardwareControl::toggleMotorDirection() {
    if (!motorRunning) {
        // Primera vez: iniciar con motor a la derecha
        startMotorRight();
        return;
    }

    // Alternar cada cierto tiempo entre los 4 estados
    if (millis() - lastMotorToggle >= MOTOR_TOGGLE_INTERVAL_MS) {
        lastMotorToggle = millis();

        switch (motorState) {
            case MOTOR_RIGHT_ACTIVE:
                // Estado 1 → Estado 2: Apagar todo (pausa)
                digitalWrite(HardwarePins::MOTOR_DIR_LEFT, LOW);
                digitalWrite(HardwarePins::MOTOR_DIR_RIGHT, LOW);
                motorState = MOTOR_PAUSE_1;
                Serial.println("[HARDWARE] Motor PAUSA (después de derecha)");
                break;

            case MOTOR_PAUSE_1:
                // Estado 2 → Estado 3: Activar izquierda
                digitalWrite(HardwarePins::MOTOR_DIR_RIGHT, LOW);
                digitalWrite(HardwarePins::MOTOR_DIR_LEFT, HIGH);
                motorState = MOTOR_LEFT_ACTIVE;
                Serial.println("[HARDWARE] Motor IZQUIERDA activado");
                break;

            case MOTOR_LEFT_ACTIVE:
                // Estado 3 → Estado 4: Apagar todo (pausa)
                digitalWrite(HardwarePins::MOTOR_DIR_LEFT, LOW);
                digitalWrite(HardwarePins::MOTOR_DIR_RIGHT, LOW);
                motorState = MOTOR_PAUSE_2;
                Serial.println("[HARDWARE] Motor PAUSA (después de izquierda)");
                break;

            case MOTOR_PAUSE_2:
                // Estado 4 → Estado 1: Activar derecha (reiniciar ciclo)
                digitalWrite(HardwarePins::MOTOR_DIR_LEFT, LOW);
                digitalWrite(HardwarePins::MOTOR_DIR_RIGHT, HIGH);
                motorState = MOTOR_RIGHT_ACTIVE;
                Serial.println("[HARDWARE] Motor DERECHA activado");
                break;
        }
    }
}

// ========================================
// Control de centrifugado
// ========================================

void HardwareControl::startCentrifuge() {
    stopMotor();  // Detener motor normal antes
    digitalWrite(HardwarePins::CENTRIFUGE, HIGH);
    centrifugeActive = true;
}

void HardwareControl::stopCentrifuge() {
    digitalWrite(HardwarePins::CENTRIFUGE, LOW);
    centrifugeActive = false;
}

// ========================================
// Control de puerta
// ========================================

void HardwareControl::lockDoor() {
    digitalWrite(HardwarePins::DOOR_MAGNET, HIGH);
    doorLocked = true;
    Serial.println("[HARDWARE] Puerta CERRADA (bloqueada)");
}

void HardwareControl::unlockDoor() {
    digitalWrite(HardwarePins::DOOR_MAGNET, LOW);
    doorLocked = false;
    Serial.println("[HARDWARE] Puerta ABIERTA (desbloqueada)");
}

// ========================================
// Botón de emergencia
// ========================================

bool HardwareControl::isEmergencyPressed() {
    // Botón con pull-up, activo en LOW
    return digitalRead(HardwarePins::EMERGENCY_BUTTON) == LOW;
}

// ========================================
// Apagado de emergencia
// ========================================

void HardwareControl::emergencyShutdown() {
    stopMotor();
    stopCentrifuge();
    closeWaterValves();
    openDrain();
    unlockDoor();
}

void HardwareControl::resetAll() {
    emergencyShutdown();
    // Mantener drenaje ABIERTO al iniciar (para vaciar cualquier agua residual)
    // closeDrain(); - Comentado para dejar el drenaje abierto al inicio
}
