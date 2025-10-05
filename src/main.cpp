#include <Arduino.h>
#include "Config.h"
#include "StateMachine.h"
#include "HardwareControl.h"
#include "SensorManager.h"
#include "NextionUI.h"

// ========================================
// INSTANCIAS GLOBALES
// ========================================

StateMachine stateMachine;
HardwareControl hardware;
SensorManager sensors;
NextionUI nextion;

// ========================================
// VARIABLES DE TIEMPO
// ========================================

unsigned long lastUIUpdate = 0;

// ========================================
// CALLBACK DE EVENTOS NEXTION
// ========================================

void handleNextionEvent(uint8_t pageId, uint8_t componentId, uint8_t eventType) {
    Serial.print("Evento Nextion - Página: ");
    Serial.print(pageId);
    Serial.print(", Componente: ");
    Serial.print(componentId);
    Serial.print(", Tipo: ");
    Serial.println(eventType);

    // Página de selección
    if (pageId == NextionConfig::PAGE_SELECTION) {
        switch (componentId) {
            case NextionConfig::BTN_PROGRAM1:
                stateMachine.selectProgram(PROGRAM_22);
                nextion.updateSelectionDisplay(stateMachine.getConfig());
                break;

            case NextionConfig::BTN_PROGRAM2:
                stateMachine.selectProgram(PROGRAM_23);
                nextion.updateSelectionDisplay(stateMachine.getConfig());
                break;

            case NextionConfig::BTN_PROGRAM3:
                stateMachine.selectProgram(PROGRAM_24);
                nextion.updateSelectionDisplay(stateMachine.getConfig());
                break;

            case NextionConfig::BTN_START:
                stateMachine.startProgram();
                nextion.showExecution();
                break;

            case NextionConfig::BTN_EDIT:
                nextion.showEdit();
                break;
        }
    }

    // Página de ejecución
    else if (pageId == NextionConfig::PAGE_EXECUTION) {
        switch (componentId) {
            case NextionConfig::BTN_PAUSE:
                if (stateMachine.getState() == STATE_PAUSED) {
                    stateMachine.resumeProgram();
                } else {
                    stateMachine.pauseProgram();
                }
                break;

            case NextionConfig::BTN_STOP:
                stateMachine.stopProgram();
                nextion.showSelection();
                break;
        }
    }

    // Página de edición
    else if (pageId == NextionConfig::PAGE_EDIT) {
        ProgramConfig& config = stateMachine.getConfig();
        uint8_t proc = config.currentProcess;

        switch (componentId) {
            case NextionConfig::BTN_PROCESS1:
                config.currentProcess = 0;
                break;

            case NextionConfig::BTN_PROCESS2:
                config.currentProcess = 1;
                break;

            case NextionConfig::BTN_PROCESS3:
                config.currentProcess = 2;
                break;

            case NextionConfig::BTN_PROCESS4:
                config.currentProcess = 3;
                break;

            case NextionConfig::BTN_PARAM_PLUS:
                // Incrementar parámetro actual
                // TODO: Implementar navegación de parámetros
                break;

            case NextionConfig::BTN_PARAM_MINUS:
                // Decrementar parámetro actual
                // TODO: Implementar navegación de parámetros
                break;

            case NextionConfig::BTN_SAVE:
                // Guardar cambios y volver a selección
                nextion.showSelection();
                nextion.updateSelectionDisplay(config);
                break;

            case NextionConfig::BTN_CANCEL:
                // Cancelar y volver a selección
                nextion.showSelection();
                break;
        }
    }

    // Página de error
    else if (pageId == NextionConfig::PAGE_ERROR) {
        if (componentId == 3) {  // BTN_REINICIAR
            hardware.resetAll();
            stateMachine.setState(STATE_SELECTION);
            nextion.showSelection();
        }
    }
}

// ========================================
// ACTUALIZACIÓN DE UI
// ========================================

void updateUI() {
    unsigned long now = millis();

    if (now - lastUIUpdate < Timing::UI_UPDATE_INTERVAL_MS) {
        return;
    }

    lastUIUpdate = now;

    SystemState state = stateMachine.getState();
    ProgramConfig& config = stateMachine.getConfig();

    // Actualizar página de ejecución si estamos en proceso
    if (state >= STATE_FILLING && state <= STATE_COOLING) {
        uint16_t phaseTime = stateMachine.getPhaseElapsedTime() / 1000;
        uint16_t totalTime = stateMachine.getTotalElapsedTime() / 1000;

        nextion.updateExecutionDisplay(
            config.programNumber,
            config.currentPhase,
            config.currentProcess,
            phaseTime,
            totalTime,
            sensors.getTemperature(),
            sensors.getWaterLevel(),
            config.centrifugeEnabled[config.currentProcess],
            config.waterType[config.currentProcess]
        );
    }
}

// ========================================
// SETUP
// ========================================

void setup() {
    // Inicializar Serial para debug
    Serial.begin(115200);
    Serial.println("\n\n========================================");
    Serial.println("Controlador de Lavadora - ESP32");
    Serial.println("========================================\n");

    // Inicializar módulos
    Serial.println("Inicializando hardware...");
    hardware.begin();

    Serial.println("Inicializando sensores...");
    sensors.begin();

    Serial.println("Inicializando pantalla Nextion...");
    nextion.begin();
    nextion.setButtonCallback(handleNextionEvent);

    Serial.println("Inicializando máquina de estados...");
    stateMachine.begin();

    // Mostrar pantalla de bienvenida
    nextion.showWelcome();

    Serial.println("\nSistema iniciado correctamente.\n");
}

// ========================================
// LOOP PRINCIPAL
// ========================================

void loop() {
    // Actualizar módulos principales
    stateMachine.update();
    hardware.update();
    sensors.update();
    nextion.update();

    // Actualizar interfaz de usuario
    updateUI();

    // Verificar emergencia
    // if (hardware.isEmergencyPressed()) {
    //     stateMachine.emergencyStop();
    //     nextion.showEmergency();
    // }

    // Sin delay() - el loop corre lo más rápido posible
}