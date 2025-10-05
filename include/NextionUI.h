#ifndef NEXTION_UI_H
#define NEXTION_UI_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include "Config.h"

// Forward declaration
struct ProgramConfig;

class NextionUI {
public:
    NextionUI();

    void begin();
    void update();

    // Navegación de páginas
    void showWelcome();
    void showSelection();
    void showExecution();
    void showEdit();
    void showError(const char* message);
    void showEmergency();

    // Actualización de página de selección
    void updateSelectionDisplay(const ProgramConfig& config);

    // Actualización de página de ejecución
    void updateExecutionDisplay(
        uint8_t program,
        uint8_t phase,
        uint8_t process,
        uint16_t phaseTime,
        uint16_t totalTime,
        float temperature,
        uint8_t waterLevel,
        bool centrifuge,
        WaterType waterType
    );

    // Actualización de página de edición
    void updateEditDisplay(
        uint8_t process,
        const char* paramName,
        const char* paramValue
    );

    // Callbacks de eventos (se configuran desde main)
    void setButtonCallback(void (*callback)(uint8_t pageId, uint8_t componentId, uint8_t eventType));

    // Utilidades
    void setText(const char* component, const char* text);
    void setNumber(const char* component, uint32_t value);
    void sendCommand(const char* cmd);

private:
    HardwareSerial* serial;
    void (*buttonCallback)(uint8_t, uint8_t, uint8_t);

    uint8_t currentPage;
    unsigned long lastUpdate;

    // Buffer para lectura de eventos
    char rxBuffer[32];
    uint8_t rxIndex;

    // Procesamiento de eventos touch
    void processSerialData();
    void parseEvent();

    // Helpers para formateo
    void formatTime(uint16_t seconds, char* buffer, size_t bufferSize);
    const char* getPhaseText(uint8_t phase);
    const char* getWaterTypeText(WaterType type);
};

#endif // NEXTION_UI_H
