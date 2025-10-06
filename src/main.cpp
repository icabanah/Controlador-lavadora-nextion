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
// VARIABLES DE TIEMPO Y ESTADO
// ========================================

unsigned long lastUIUpdate = 0;
SystemState lastDisplayedState = STATE_INIT;

// ========================================
// VARIABLES DE EDICIÓN
// ========================================

enum ParameterType {
    PARAM_NIVEL,
    PARAM_TEMP,
    PARAM_TIEMPO,
    PARAM_CENTRIF,
    PARAM_AGUA
};

struct EditState {
    uint8_t currentTanda;        // 0-3
    ParameterType currentParam;  // Parámetro seleccionado
    bool editingValue;           // true = editando valor, false = seleccionando parámetro
    ProgramConfig backupConfig;  // Backup para cancelar
} editState;

// ========================================
// FUNCIONES DE EDICIÓN
// ========================================

void enterEditMode() {
    ProgramConfig& config = stateMachine.getConfig();

    // Guardar backup
    editState.backupConfig = config;
    editState.currentTanda = 0;
    editState.currentParam = PARAM_NIVEL;
    editState.editingValue = false;

    Serial.println("=== Entrando a modo edición ===");
}

void updateEditDisplay() {
    ProgramConfig& config = stateMachine.getConfig();
    uint8_t tanda = editState.currentTanda;

    // Actualizar valores del panel derecho
    char buffer[32];

    snprintf(buffer, sizeof(buffer), "%d", config.waterLevel[tanda]);
    nextion.setText("val_nivel", buffer);

    snprintf(buffer, sizeof(buffer), "%d", config.temperature[tanda]);
    nextion.setText("val_temp", buffer);

    snprintf(buffer, sizeof(buffer), "%d", config.time[tanda]);
    nextion.setText("val_tiempo", buffer);

    nextion.setText("val_centrif", config.centrifugeEnabled[tanda] ? "Si" : "No");
    nextion.setText("val_agua", config.waterType[tanda] == WATER_HOT ? "Caliente" : "Fria");

    // Actualizar botones de tanda (desactivar tandas no usadas en P22/P23)
    uint8_t totalTandas = config.totalProcesses;
    for (int i = 0; i < 4; i++) {
        char tandaName[16];
        snprintf(tandaName, sizeof(tandaName), "tanda%d", i + 1);

        if (i < totalTandas) {
            // Activa: resaltar si es la seleccionada
            nextion.setNumber(tandaName, (i == tanda) ? 1 : 0);
        } else {
            // Inactiva: valor -1 o deshabilitada
            nextion.setNumber(tandaName, 2);  // Estado "deshabilitado"
        }
    }

    // Mostrar parámetro actual en edición
    const char* paramName = "";
    char paramValue[32] = "";

    switch (editState.currentParam) {
        case PARAM_NIVEL:
            paramName = "Nivel de Agua";
            snprintf(paramValue, sizeof(paramValue), "%d", config.waterLevel[tanda]);
            break;
        case PARAM_TEMP:
            paramName = "Temperatura";
            snprintf(paramValue, sizeof(paramValue), "%d°C", config.temperature[tanda]);
            break;
        case PARAM_TIEMPO:
            paramName = "Tiempo";
            snprintf(paramValue, sizeof(paramValue), "%d min", config.time[tanda]);
            break;
        case PARAM_CENTRIF:
            paramName = "Centrifugado";
            snprintf(paramValue, sizeof(paramValue), "%s", config.centrifugeEnabled[tanda] ? "Si" : "No");
            break;
        case PARAM_AGUA:
            paramName = "Tipo de Agua";
            snprintf(paramValue, sizeof(paramValue), "%s", config.waterType[tanda] == WATER_HOT ? "Caliente" : "Fria");
            break;
    }

    nextion.updateEditDisplay(tanda, paramName, paramValue);
}

void incrementCurrentParameter() {
    ProgramConfig& config = stateMachine.getConfig();
    uint8_t tanda = editState.currentTanda;

    switch (editState.currentParam) {
        case PARAM_NIVEL:
            if (config.waterLevel[tanda] < Limits::MAX_WATER_LEVEL)
                config.waterLevel[tanda]++;
            break;
        case PARAM_TEMP:
            if (config.temperature[tanda] < Limits::MAX_TEMPERATURE)
                config.temperature[tanda]++;
            break;
        case PARAM_TIEMPO:
            if (config.time[tanda] < Limits::MAX_TIME)
                config.time[tanda]++;
            break;
        case PARAM_CENTRIF:
            config.centrifugeEnabled[tanda] = !config.centrifugeEnabled[tanda];
            break;
        case PARAM_AGUA:
            config.waterType[tanda] = (config.waterType[tanda] == WATER_HOT) ? WATER_COLD : WATER_HOT;
            break;
    }

    updateEditDisplay();
}

void decrementCurrentParameter() {
    ProgramConfig& config = stateMachine.getConfig();
    uint8_t tanda = editState.currentTanda;

    switch (editState.currentParam) {
        case PARAM_NIVEL:
            if (config.waterLevel[tanda] > Limits::MIN_WATER_LEVEL)
                config.waterLevel[tanda]--;
            break;
        case PARAM_TEMP:
            if (config.temperature[tanda] > Limits::MIN_TEMPERATURE)
                config.temperature[tanda]--;
            break;
        case PARAM_TIEMPO:
            if (config.time[tanda] > Limits::MIN_TIME)
                config.time[tanda]--;
            break;
        case PARAM_CENTRIF:
            config.centrifugeEnabled[tanda] = !config.centrifugeEnabled[tanda];
            break;
        case PARAM_AGUA:
            config.waterType[tanda] = (config.waterType[tanda] == WATER_HOT) ? WATER_COLD : WATER_HOT;
            break;
    }

    updateEditDisplay();
}

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
                enterEditMode();
                nextion.showEdit();
                updateEditDisplay();
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

        switch (componentId) {
            // Botones de selección de tanda
            case NextionConfig::BTN_PROCESS1:
                editState.currentTanda = 0;
                updateEditDisplay();
                break;

            case NextionConfig::BTN_PROCESS2:
                if (config.totalProcesses > 1) {
                    editState.currentTanda = 1;
                    updateEditDisplay();
                }
                break;

            case NextionConfig::BTN_PROCESS3:
                if (config.totalProcesses > 2) {
                    editState.currentTanda = 2;
                    updateEditDisplay();
                }
                break;

            case NextionConfig::BTN_PROCESS4:
                if (config.totalProcesses > 3) {
                    editState.currentTanda = 3;
                    updateEditDisplay();
                }
                break;

            // Botones del panel derecho (selección de parámetro)
            case NextionConfig::BTN_PANEL_NIVEL:
                editState.currentParam = PARAM_NIVEL;
                updateEditDisplay();
                break;

            case NextionConfig::BTN_PANEL_TEMP:
                editState.currentParam = PARAM_TEMP;
                updateEditDisplay();
                break;

            case NextionConfig::BTN_PANEL_TIEMPO:
                editState.currentParam = PARAM_TIEMPO;
                updateEditDisplay();
                break;

            case NextionConfig::BTN_PANEL_CENTRIF:
                editState.currentParam = PARAM_CENTRIF;
                updateEditDisplay();
                break;

            case NextionConfig::BTN_PANEL_AGUA:
                editState.currentParam = PARAM_AGUA;
                updateEditDisplay();
                break;

            // Botones de incremento/decremento
            case NextionConfig::BTN_PARAM_PLUS:
                incrementCurrentParameter();
                break;

            case NextionConfig::BTN_PARAM_MINUS:
                decrementCurrentParameter();
                break;

            // Botón Guardar (doble clic: primero guarda, segundo sale)
            case NextionConfig::BTN_SAVE:
                if (!editState.editingValue) {
                    // Primera vez: guardar configuración
                    editState.editingValue = true;
                    Serial.println("[EDIT] Configuración guardada");
                } else {
                    // Segunda vez: salir a página de selección
                    editState.editingValue = false;
                    stateMachine.setState(STATE_SELECTION);
                    nextion.showSelection();
                    nextion.updateSelectionDisplay(config);
                    Serial.println("[EDIT] Volviendo a página de selección");
                }
                break;

            // Botón Cancelar (restaura backup y vuelve)
            case NextionConfig::BTN_CANCEL:
                config = editState.backupConfig;
                editState.editingValue = false;
                stateMachine.setState(STATE_SELECTION);
                nextion.showSelection();
                nextion.updateSelectionDisplay(config);
                Serial.println("[EDIT] Cambios cancelados");
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

    SystemState state = stateMachine.getState();
    ProgramConfig& config = stateMachine.getConfig();

    // Cambiar página de Nextion si el estado cambió
    if (state != lastDisplayedState) {
        lastDisplayedState = state;

        switch (state) {
            case STATE_WELCOME:
                nextion.showWelcome();
                Serial.println("UI: Mostrando página de bienvenida");
                break;

            case STATE_SELECTION:
                nextion.showSelection();
                nextion.updateSelectionDisplay(config);
                Serial.println("UI: Mostrando página de selección");
                break;

            case STATE_FILLING:
            case STATE_WASHING:
            case STATE_DRAINING:
            case STATE_SPINNING:
            case STATE_COOLING:
                nextion.showExecution();
                Serial.println("UI: Mostrando página de ejecución");
                break;

            case STATE_PAUSED:
                Serial.println("UI: Programa pausado");
                break;

            case STATE_COMPLETED:
                Serial.println("UI: Programa completado");
                nextion.showSelection();
                break;

            case STATE_ERROR:
                nextion.showError("Error del sistema");
                Serial.println("UI: Mostrando página de error");
                break;

            case STATE_EMERGENCY:
                nextion.showEmergency();
                Serial.println("UI: EMERGENCIA ACTIVADA");
                break;

            default:
                break;
        }
    }

    // Actualizar datos solo cada intervalo
    if (now - lastUIUpdate < Timing::UI_UPDATE_INTERVAL_MS) {
        return;
    }

    lastUIUpdate = now;

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

    Serial.println("\nSistema iniciado correctamente.\n");
    Serial.println("Esperando 3 segundos para pasar a selección...");
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