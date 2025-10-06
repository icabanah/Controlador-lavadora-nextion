#include "HardwareControl.h"

HardwareControl::HardwareControl()
    : doorLocked(false),
      drainOpen(false),
      motorRunning(false),
      centrifugeActive(false),
      lastMotorToggle(0),
      motorDirectionLeft(true) {}

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
    // NOTA: Comentado temporalmente - el botón de emergencia puede estar flotando
    // if (isEmergencyPressed()) {
    //     emergencyShutdown();
    // }
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
    motorDirectionLeft = true;
    lastMotorToggle = millis();
    Serial.println("[HARDWARE] Motor IZQUIERDA activado");
}

void HardwareControl::startMotorRight() {
    digitalWrite(HardwarePins::MOTOR_DIR_LEFT, LOW);
    digitalWrite(HardwarePins::MOTOR_DIR_RIGHT, HIGH);
    motorRunning = true;
    motorDirectionLeft = false;
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
        startMotorLeft();
        return;
    }

    // Alternar cada cierto tiempo
    if (millis() - lastMotorToggle >= MOTOR_TOGGLE_INTERVAL_MS) {
        if (motorDirectionLeft) {
            startMotorRight();
        } else {
            startMotorLeft();
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
    closeDrain();
}
