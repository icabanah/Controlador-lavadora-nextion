#include "NextionUI.h"
#include "StateMachine.h"

NextionUI::NextionUI()
    : serial(&Serial2),
      buttonCallback(nullptr),
      currentPage(0),
      lastUpdate(0),
      rxIndex(0) {
    memset(rxBuffer, 0, sizeof(rxBuffer));
}

// ========================================
// Inicialización
// ========================================

void NextionUI::begin() {
    serial->begin(NextionConfig::BAUD_RATE, SERIAL_8N1,
                  HardwarePins::NEXTION_RX, HardwarePins::NEXTION_TX);

    // Espera no bloqueante: el Nextion se inicializa en ~500ms
    // La primera actualización lo manejará automáticamente

    // Limpiar buffer
    unsigned long startTime = millis();
    while (millis() - startTime < 100) {
        if (serial->available()) {
            serial->read();
        }
    }

    // Comando de inicialización (opcional)
    sendCommand("bkcmd=0");  // Desactivar respuestas automáticas
}

// ========================================
// Actualización periódica
// ========================================

void NextionUI::update() {
    processSerialData();
}

// ========================================
// Navegación de páginas
// ========================================

void NextionUI::showWelcome() {
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "page %d", NextionConfig::PAGE_WELCOME);
    sendCommand(cmd);
    currentPage = NextionConfig::PAGE_WELCOME;
}

void NextionUI::showSelection() {
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "page %d", NextionConfig::PAGE_SELECTION);
    sendCommand(cmd);
    currentPage = NextionConfig::PAGE_SELECTION;
}

void NextionUI::showExecution() {
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "page %d", NextionConfig::PAGE_EXECUTION);
    sendCommand(cmd);
    currentPage = NextionConfig::PAGE_EXECUTION;
}

void NextionUI::showEdit() {
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "page %d", NextionConfig::PAGE_EDIT);
    sendCommand(cmd);
    currentPage = NextionConfig::PAGE_EDIT;
}

void NextionUI::showError(const char* message) {
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "page %d", NextionConfig::PAGE_ERROR);
    sendCommand(cmd);
    setText("mensaje", message);
    currentPage = NextionConfig::PAGE_ERROR;
}

void NextionUI::showEmergency() {
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "page %d", NextionConfig::PAGE_EMERGENCY);
    sendCommand(cmd);
    currentPage = NextionConfig::PAGE_EMERGENCY;
}

// ========================================
// Actualización de displays
// ========================================

void NextionUI::updateSelectionDisplay(const ProgramConfig& config) {
    char buffer[32];


    // Mostrar programa seleccionado
    snprintf(buffer, sizeof(buffer), "P%d", config.programNumber);
    setText("progr_sel", buffer);
    // Serial.printf("  progr_sel = %s\n", buffer);

    uint8_t proc = config.currentProcess;

    // Nivel de agua (como texto)
    snprintf(buffer, sizeof(buffer), "%d", config.waterLevel[proc]);
    setText("val_nivel", buffer);
    // Serial.printf("  val_nivel = %s\n", buffer);

    // Temperatura (como texto)
    snprintf(buffer, sizeof(buffer), "%d", config.temperature[proc]);
    setText("val_temp", buffer);
    // Serial.printf("  val_temp = %s\n", buffer);

    // Tiempo (como texto)
    snprintf(buffer, sizeof(buffer), "%d", config.time[proc]);
    setText("val_tiempo", buffer);
    // Serial.printf("  val_tiempo = %s\n", buffer);

    // Centrifugado
    const char* centrif = config.centrifugeEnabled[proc] ? "Si" : "No";
    setText("val_centrif", centrif);
    // Serial.printf("  val_centrif = %s\n", centrif);

    // Tipo de agua
    const char* agua = getWaterTypeText(config.waterType[proc]);
    setText("val_agua", agua);
    // Serial.printf("  val_agua = %s\n", agua);
}

void NextionUI::updateExecutionDisplay(
    uint8_t program,
    uint8_t phase,
    uint8_t process,
    uint16_t phaseTime,
    uint16_t totalTime,
    float temperature,
    uint8_t waterLevel,
    bool centrifuge,
    WaterType waterType)
{
    char buffer[32];

    // Programa
    snprintf(buffer, sizeof(buffer), "P%d", program);
    setText("progr_ejec", buffer);

    // Fase
    setText("fase_ejec", getPhaseText(phase));

    // Proceso/Tanda (como texto)
    snprintf(buffer, sizeof(buffer), "%d", process + 1);
    setText("tanda_ejec", buffer);

    // Tiempo de fase
    formatTime(phaseTime, buffer, sizeof(buffer));
    setText("tiempo_ejec", buffer);

    // Tiempo total
    formatTime(totalTime, buffer, sizeof(buffer));
    setText("tiempo_total", buffer);

    // Temperatura
    snprintf(buffer, sizeof(buffer), "%.1f C", temperature);
    setText("temp_ejec", buffer);

    // Nivel (como texto)
    snprintf(buffer, sizeof(buffer), "%d", waterLevel);
    setText("nivel_ejec", buffer);

    // Barras de progreso (estas Si usan .val porque son progress bars)
    setNumber("barra_nivel", (waterLevel * 100) / 4);

    // Barra de temperatura (0-100°C mapeado a 0-100%)
    uint8_t tempPercent = (uint8_t)constrain(temperature, 0, 100);
    setNumber("barra_temp", tempPercent);


    // Centrifugado
    setText("centrif_ejec", centrifuge ? "Si" : "No");

    // Tipo de agua
    setText("agua_ejec", getWaterTypeText(waterType));
}

void NextionUI::updateEditDisplay(
    uint8_t process,
    const char* paramName,
    const char* paramValue)
{
    char buffer[16];

    // Resaltar proceso activo (los botones de tanda pueden usar .val para estado)
    for (int i = 1; i <= 4; i++) {
        snprintf(buffer, sizeof(buffer), "tanda%d", i);
        setNumber(buffer, (i == process + 1) ? 1 : 0);  // 1=activo, 0=inactivo
    }

    // Mostrar parámetro y valor (como texto)
    setText("param", paramName);
    setText("param_value", paramValue);
}

// ========================================
// Callback
// ========================================

void NextionUI::setButtonCallback(void (*callback)(uint8_t, uint8_t, uint8_t)) {
    buttonCallback = callback;
}

// ========================================
// Utilidades
// ========================================

void NextionUI::setText(const char* component, const char* text) {
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "%s.txt=\"%s\"", component, text);
    // Serial.printf("[NEXTION] Enviando: %s\n", cmd);
    sendCommand(cmd);
}

void NextionUI::setNumber(const char* component, uint32_t value) {
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "%s.val=%lu", component, value);
    // Serial.printf("[NEXTION] Enviando: %s\n", cmd);
    sendCommand(cmd);
}

void NextionUI::setEnabled(const char* component, bool enabled) {
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "%s.en=%d", component, enabled ? 1 : 0);
    // Serial.printf("[NEXTION] Enviando: %s\n", cmd);
    sendCommand(cmd);
}

void NextionUI::setEnabledById(uint8_t componentId, bool enabled) {
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "tsw %d,%d", componentId, enabled ? 1 : 0);
    // Serial.printf("[NEXTION] Enviando: %s\n", cmd);
    sendCommand(cmd);
}

void NextionUI::setBackgroundColor(const char* component, uint16_t color) {
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "%s.bco=%u", component, color);
    // Serial.printf("[NEXTION] Enviando: %s\n", cmd);
    sendCommand(cmd);
}

void NextionUI::sendCommand(const char* cmd) {
    serial->print(cmd);
    serial->write(0xFF);
    serial->write(0xFF);
    serial->write(0xFF);
}

// ========================================
// Procesamiento de eventos
// ========================================

void NextionUI::processSerialData() {
    static uint8_t ffCount = 0;

    while (serial->available()) {
        uint8_t byte = serial->read();

        // Detectar fin de mensaje (3 bytes 0xFF)
        if (byte == 0xFF) {
            ffCount++;

            if (ffCount >= 3) {
                parseEvent();
                rxIndex = 0;
                memset(rxBuffer, 0, sizeof(rxBuffer));
                ffCount = 0;
            }
        } else {
            ffCount = 0;  // Reset contador si no es 0xFF

            // Almacenar byte en buffer
            if (rxIndex < sizeof(rxBuffer) - 1) {
                rxBuffer[rxIndex++] = byte;
            }
        }
    }
}

void NextionUI::parseEvent() {
    if (rxIndex == 0) return;

    // Evento touch: 0x65 [pageId] [componentId] [eventType]
    if (rxBuffer[0] == 0x65 && rxIndex >= 4) {
        uint8_t pageId = rxBuffer[1];
        uint8_t componentId = rxBuffer[2];
        uint8_t eventType = rxBuffer[3];

        Serial.print("Nextion Event: Page=");
        Serial.print(pageId);
        Serial.print(", Comp=");
        Serial.print(componentId);
        Serial.print(", Type=");
        Serial.println(eventType);

        if (buttonCallback != nullptr) {
            buttonCallback(pageId, componentId, eventType);
        }
    }
}

// ========================================
// Helpers de formateo
// ========================================

void NextionUI::formatTime(uint16_t seconds, char* buffer, size_t bufferSize) {
    uint16_t minutes = seconds / 60;
    uint16_t secs = seconds % 60;
    snprintf(buffer, bufferSize, "%02d:%02d", minutes, secs);
}

const char* NextionUI::getPhaseText(uint8_t phase) {
    switch (phase) {
        case PHASE_FILLING:   return "Llenado";
        case PHASE_WASHING:   return "Lavado";
        case PHASE_DRAINING:  return "Drenaje";
        case PHASE_SPINNING:  return "Centrifugado";
        case PHASE_COOLING:   return "Enfriamiento";
        default:              return "Desconocido";
    }
}

const char* NextionUI::getWaterTypeText(WaterType type) {
    return (type == WATER_HOT) ? "Caliente" : "Fria";
}
