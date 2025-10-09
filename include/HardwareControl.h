#ifndef HARDWARE_CONTROL_H
#define HARDWARE_CONTROL_H

#include <Arduino.h>
#include "Config.h"

class HardwareControl {
public:
    HardwareControl();

    void begin();
    void update();

    // Control de válvulas
    void openColdWater();
    void openHotWater();
    void closeWaterValves();
    void openDrain();
    void closeDrain();

    // Control de motor
    void startMotorLeft();
    void startMotorRight();
    void stopMotor();
    void toggleMotorDirection();  // Alterna izquierda/derecha

    // Control de centrifugado
    void startCentrifuge();
    void stopCentrifuge();

    // Control de puerta
    void lockDoor();
    void unlockDoor();

    // Lectura de botón de emergencia
    bool isEmergencyPressed();

    // Apagado total de seguridad
    void emergencyShutdown();
    void resetAll();

private:
    bool doorLocked;
    bool drainOpen;
    bool motorRunning;
    bool centrifugeActive;
    unsigned long lastMotorToggle;

    // Estados del motor (secuencia de 4 pasos)
    enum MotorState : uint8_t {
        MOTOR_RIGHT_ACTIVE = 0,   // Derecha activada
        MOTOR_PAUSE_1 = 1,         // Pausa después de derecha
        MOTOR_LEFT_ACTIVE = 2,     // Izquierda activada
        MOTOR_PAUSE_2 = 3          // Pausa después de izquierda
    };
    MotorState motorState;

    // Antirrebote para botón de emergencia
    bool emergencyButtonState;           // Estado actual del botón (después de debounce)
    bool lastEmergencyButtonRead;        // Última lectura del pin
    unsigned long lastEmergencyDebounceTime;  // Tiempo de última transición
    bool emergencyTriggered;             // Flag de emergencia ya activada

    // Configuración de tiempos
    static constexpr uint16_t MOTOR_RIGHT_TIME_MS = 7000;   // Tiempo girando a la derecha
    static constexpr uint16_t MOTOR_PAUSE_TIME_MS = 4000;   // Tiempo de pausa entre giros
    static constexpr uint16_t MOTOR_LEFT_TIME_MS = 6000;    // Tiempo girando a la izquierda
    static constexpr uint8_t EMERGENCY_DEBOUNCE_MS = 200;   // Tiempo de antirrebote
};

#endif // HARDWARE_CONTROL_H
