#include <Arduino.h>
#include "Config.h"
#include "StateMachine.h"
#include "HardwareControl.h"
#include "SensorManager.h"
#include "NextionUI.h"
#include "Storage.h"

// ========================================
// INSTANCIAS GLOBALES
// ========================================

StateMachine stateMachine;
HardwareControl hardware;
SensorManager sensors;
NextionUI nextion;
Storage storage;

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
// FUNCIONES DE SELECCIÓN
// ========================================

void updateProgramButtons(uint8_t selectedProgram) {
    // IDs de botones de programa
    const uint8_t programIds[3] = {
        NextionConfig::BTN_PROGRAM1,  // ID 1
        NextionConfig::BTN_PROGRAM2,  // ID 2
        NextionConfig::BTN_PROGRAM3   // ID 3
    };

    const uint8_t programNumbers[3] = {22, 23, 24};

    // Actualizar cada botón
    for (int i = 0; i < 3; i++) {
        char btnName[16];
        snprintf(btnName, sizeof(btnName), "btnPrograma%d", i + 1);

        if (programNumbers[i] == selectedProgram) {
            // Botón seleccionado: color activo
            nextion.setBackgroundColor(btnName, NextionConfig::COLOR_ACTIVE);
        } else {
            // Botón no seleccionado: color inactivo
            nextion.setBackgroundColor(btnName, NextionConfig::COLOR_INACTIVE);
        }
    }
}

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
}

void updateEditDisplay() {
    ProgramConfig& config = stateMachine.getConfig();
    uint8_t tanda = editState.currentTanda;

    // Actualizar número de programa en edición
    char buffer[32];
        snprintf(buffer, sizeof(buffer), "P%d", config.programNumber);
    nextion.setText("progr_sel", buffer);

    // Actualizar valores del panel derecho
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

    // IDs de botones de tanda (definidos en Config.h)
    const uint8_t tandaIds[4] = {
        NextionConfig::BTN_PROCESS1,
        NextionConfig::BTN_PROCESS2,
        NextionConfig::BTN_PROCESS3,
        NextionConfig::BTN_PROCESS4
    };

    for (int i = 0; i < 4; i++) {
        char tandaName[16];
        snprintf(tandaName, sizeof(tandaName), "tanda%d", i + 1);

        if (i < totalTandas) {
            // Tanda disponible: habilitar con tsw
            nextion.setEnabledById(tandaIds[i], true);

            // Cambiar color según si está seleccionada o no
            if (i == tanda) {
                nextion.setBackgroundColor(tandaName, NextionConfig::COLOR_ACTIVE);
                nextion.setNumber(tandaName, 1);  // Valor seleccionado
            } else {
                nextion.setBackgroundColor(tandaName, NextionConfig::COLOR_INACTIVE);
                nextion.setNumber(tandaName, 0);  // Valor no seleccionado
            }
        } else {
            // Tanda no disponible: deshabilitar con tsw y color deshabilitado
            nextion.setEnabledById(tandaIds[i], false);
            nextion.setBackgroundColor(tandaName, NextionConfig::COLOR_DISABLED);
            nextion.setNumber(tandaName, 0);
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

void nextParameter() {
    // Navegar al siguiente parámetro
    switch (editState.currentParam) {
        case PARAM_NIVEL:
            editState.currentParam = PARAM_TEMP;
            break;
        case PARAM_TEMP:
            editState.currentParam = PARAM_TIEMPO;
            break;
        case PARAM_TIEMPO:
            editState.currentParam = PARAM_CENTRIF;
            break;
        case PARAM_CENTRIF:
            editState.currentParam = PARAM_AGUA;
            break;
        case PARAM_AGUA:
            editState.currentParam = PARAM_NIVEL;  // Ciclo
            break;
    }
    updateEditDisplay();
}

void prevParameter() {
    // Navegar al parámetro anterior
    switch (editState.currentParam) {
        case PARAM_NIVEL:
            editState.currentParam = PARAM_AGUA;  // Ciclo
            break;
        case PARAM_TEMP:
            editState.currentParam = PARAM_NIVEL;
            break;
        case PARAM_TIEMPO:
            editState.currentParam = PARAM_TEMP;
            break;
        case PARAM_CENTRIF:
            editState.currentParam = PARAM_TIEMPO;
            break;
        case PARAM_AGUA:
            editState.currentParam = PARAM_CENTRIF;
            break;
    }
    updateEditDisplay();
}

// ========================================
// CALLBACK DE EVENTOS NEXTION
// ========================================

void handleNextionEvent(uint8_t pageId, uint8_t componentId, uint8_t eventType) {
    // Serial.print("Evento Nextion - Página: ");
    // Serial.print(pageId);
    // Serial.print(", Componente: ");
    // Serial.print(componentId);
    // Serial.print(", Tipo: ");
    // Serial.println(eventType);

    // Página de selección
    if (pageId == NextionConfig::PAGE_SELECTION) {
        switch (componentId) {
            case NextionConfig::BTN_PROGRAM1:
                // Cargar configuración guardada si existe
                if (!storage.loadProgram(22, stateMachine.getConfig())) {
                    // Si no existe, usar valores por defecto
                    stateMachine.getConfig().setDefaults(PROGRAM_22);
                }
                updateProgramButtons(22);  // Resaltar botón seleccionado
                nextion.updateSelectionDisplay(stateMachine.getConfig());
                break;

            case NextionConfig::BTN_PROGRAM2:
                // Cargar configuración guardada si existe
                if (!storage.loadProgram(23, stateMachine.getConfig())) {
                    stateMachine.getConfig().setDefaults(PROGRAM_23);
                }
                updateProgramButtons(23);  // Resaltar botón seleccionado
                nextion.updateSelectionDisplay(stateMachine.getConfig());
                break;

            case NextionConfig::BTN_PROGRAM3:
                // Cargar configuración guardada si existe
                if (!storage.loadProgram(24, stateMachine.getConfig())) {
                    stateMachine.getConfig().setDefaults(PROGRAM_24);
                }
                updateProgramButtons(24);  // Resaltar botón seleccionado
                nextion.updateSelectionDisplay(stateMachine.getConfig());
                break;

            case NextionConfig::BTN_START:
                stateMachine.startProgram();
                sensors.startMonitoring();  // ACTIVAR sensores al iniciar programa
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
                    // Cambiar texto del botón a "Pausar"
                    nextion.setText("btnPausar", "Pausar");
                    stateMachine.resumeProgram();
                    sensors.startMonitoring();  // REACTIVAR sensores al reanudar
                } else {
                    // Cambiar texto del botón a "Reiniciar"
                    nextion.setText("btnPausar", "Reiniciar");
                    stateMachine.pauseProgram();
                    sensors.stopMonitoring();  // DESACTIVAR sensores al pausar
                }
                break;

            case NextionConfig::BTN_STOP:
                stateMachine.stopProgram();
                sensors.stopMonitoring();  // DESACTIVAR sensores al detener

                // Cargar programa por defecto (P22) desde storage
                if (!storage.loadProgram(22, stateMachine.getConfig())) {
                    stateMachine.getConfig().setDefaults(PROGRAM_22);
                }
                stateMachine.setState(STATE_SELECTION);

                nextion.showSelection();
                updateProgramButtons(22);  // Resaltar botón P22
                nextion.updateSelectionDisplay(stateMachine.getConfig());
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
                // Serial.println("[EDIT] Tanda 1 seleccionada");
                break;

            case NextionConfig::BTN_PROCESS2:
                if (config.totalProcesses > 1) {
                    editState.currentTanda = 1;
                    updateEditDisplay();
                } else {
                    // Serial.println("[EDIT] Tanda 2 no disponible para este programa");
                }
                break;

            case NextionConfig::BTN_PROCESS3:
                if (config.totalProcesses > 2) {
                    editState.currentTanda = 2;
                    updateEditDisplay();
                    // Serial.println("[EDIT] Tanda 3 seleccionada");
                } else {
                    // Serial.println("[EDIT] Tanda 3 no disponible para este programa");
                }
                break;

            case NextionConfig::BTN_PROCESS4:
                if (config.totalProcesses > 3) {
                    editState.currentTanda = 3;
                    updateEditDisplay();
                    // Serial.println("[EDIT] Tanda 4 seleccionada");
                } else {
                    // Serial.println("[EDIT] Tanda 4 no disponible para este programa");
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

            // Botones de navegación de parámetros
            case NextionConfig::BTN_PARAM_NEXT:
                nextParameter();
                // Serial.println("[EDIT] Siguiente parámetro");
                break;

            case NextionConfig::BTN_PARAM_PREV:
                prevParameter();
                // Serial.println("[EDIT] Parámetro anterior");
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
                // Serial.println("[EDIT] ========== BOTÓN GUARDAR PRESIONADO ==========");
                if (!editState.editingValue) {
                    // Primera vez: guardar configuración en memoria persistente
                    editState.editingValue = true;
                    // Serial.printf("[EDIT] Guardando programa %d...\n", config.programNumber);
                    storage.saveProgram(config.programNumber, config);
                    // Serial.println("[EDIT] Configuración guardada en memoria");
                } else {
                    // Segunda vez: salir a página de selección
                    editState.editingValue = false;
                    stateMachine.setState(STATE_SELECTION);
                    nextion.showSelection();
                    updateProgramButtons(config.programNumber);  // Resaltar programa
                    nextion.updateSelectionDisplay(config);
                    // Serial.println("[EDIT] Volviendo a página de selección");
                }
                break;

            // Botón Cancelar (restaura backup y vuelve)
            case NextionConfig::BTN_CANCEL:
                config = editState.backupConfig;
                editState.editingValue = false;
                stateMachine.setState(STATE_SELECTION);
                nextion.showSelection();
                updateProgramButtons(config.programNumber);  // Resaltar programa
                nextion.updateSelectionDisplay(config);
                // Serial.println("[EDIT] Cambios cancelados");
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
                // Serial.println("UI: Mostrando página de bienvenida");
                break;

            case STATE_SELECTION:
                nextion.showSelection();
                updateProgramButtons(config.programNumber);  // Resaltar programa actual
                nextion.updateSelectionDisplay(config);
                // Serial.println("UI: Mostrando página de selección");
                break;

            case STATE_FILLING:
            case STATE_WASHING:
            case STATE_DRAINING:
            case STATE_SPINNING:
            case STATE_COOLING:
                nextion.showExecution();
                // Serial.println("UI: Mostrando página de ejecución");
                break;

            case STATE_PAUSED:
                // Serial.println("UI: Programa pausado");
                break;

            case STATE_COMPLETED:
                // Serial.println("UI: Programa completado");
                sensors.stopMonitoring();  // DESACTIVAR sensores al completar

                // Cargar programa por defecto (P22) desde storage
                if (!storage.loadProgram(22, stateMachine.getConfig())) {
                    stateMachine.getConfig().setDefaults(PROGRAM_22);
                }
                stateMachine.setState(STATE_SELECTION);

                nextion.showSelection();
                updateProgramButtons(22);  // Resaltar botón P22
                nextion.updateSelectionDisplay(stateMachine.getConfig());
                break;

            case STATE_ERROR:
                sensors.stopMonitoring();  // DESACTIVAR sensores en error
                nextion.showError("Error del sistema");
                // Serial.println("UI: Mostrando página de error");
                break;

            case STATE_EMERGENCY:
                sensors.stopMonitoring();  // DESACTIVAR sensores en emergencia
                nextion.showEmergency();
                // Serial.println("UI: EMERGENCIA ACTIVADA");
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

    // Actualizar página de ejecución si estamos en proceso o pausado
    if ((state >= STATE_FILLING && state <= STATE_COOLING) || state == STATE_PAUSED) {
        // Mostrar tiempo restante si está en fase de lavado o pausado, sino 0
        uint16_t phaseTime = (stateMachine.isTimerActive() || state == STATE_PAUSED)
            ? stateMachine.getPhaseRemainingTime() / 1000
            : 0;
        // Tiempo total del programa (valor fijo calculado, no cambia durante ejecución)
        uint16_t totalTime = stateMachine.getTotalProgramTime();

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

        // Parpadeo del temporizador cuando está pausado (alternar visibilidad)
        static bool blinkState = false;
        if (state == STATE_PAUSED) {
            blinkState = !blinkState;
            // Alternar visibilidad del componente tiempo_ejec
            nextion.sendCommand(blinkState ? "vis tiempo_ejec,1" : "vis tiempo_ejec,0");
        } else {
            // Asegurar que esté visible cuando no está pausado
            nextion.sendCommand("vis tiempo_ejec,1");
        }
    }
}

// ========================================
// SETUP
// ========================================

void setup() {
    // Inicializar Serial para debug
    Serial.begin(115200);

    // Esperar a que se conecte el monitor serial (5 segundos)
    delay(5000);

    // Inicializar almacenamiento persistente
    // Serial.println("Inicializando almacenamiento...");
    storage.begin();
    storage.debugPrintAll();  // Debug: mostrar qué hay en memoria

    // Inicializar módulos
    // Serial.println("Inicializando hardware...");
    hardware.begin();

    // Serial.println("Inicializando sensores...");
    sensors.begin();

    // Serial.println("Inicializando pantalla Nextion...");
    nextion.begin();
    nextion.setButtonCallback(handleNextionEvent);

    // Serial.println("Inicializando máquina de estados...");
    stateMachine.begin();

    // Cargar configuración guardada del programa por defecto (P22)
    // Serial.println("Cargando configuración guardada...");
    if (storage.loadProgram(22, stateMachine.getConfig())) {
        // Serial.println("Configuración de P22 cargada desde memoria");
    } else {
        // Serial.println("Usando configuración por defecto de P22");
    }

    // Serial.println("\nSistema iniciado correctamente.\n");
    // Serial.println("Esperando 3 segundos para pasar a selección...");
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

    // Verificar emergencia (activar/desactivar según estado del botón)
    static bool wasEmergencyActive = false;
    bool isEmergencyActive = hardware.isEmergencyPressed();

    if (isEmergencyActive && !wasEmergencyActive) {
        // Botón de emergencia PRESIONADO (flanco ascendente)
        stateMachine.emergencyStop();
        sensors.stopMonitoring();
        nextion.showEmergency();
        wasEmergencyActive = true;
        // Serial.println("[EMERGENCY] Botón de emergencia ACTIVADO");
    }
    else if (!isEmergencyActive && wasEmergencyActive) {
        // Botón de emergencia SOLTADO (flanco descendente)
        // Volver a página de selección y resetear sistema
        // Cargar configuración guardada de P22 (NO llamar selectProgram antes)
        if (!storage.loadProgram(22, stateMachine.getConfig())) {
            // Solo si no hay configuración guardada, usar valores por defecto
            stateMachine.getConfig().setDefaults(PROGRAM_22);
        }
        stateMachine.setState(STATE_SELECTION);
        nextion.showSelection();
        updateProgramButtons(22);
        nextion.updateSelectionDisplay(stateMachine.getConfig());
        wasEmergencyActive = false;
        // Serial.println("[EMERGENCY] Botón de emergencia DESACTIVADO - Sistema reseteado");
    }

    // Sin delay() - el loop corre lo más rápido posible
}