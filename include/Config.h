#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ========================================
// CONFIGURACIÓN DE PINES HARDWARE
// ========================================

namespace HardwarePins
{
    // Entrada de emergencia
    constexpr uint8_t EMERGENCY_BUTTON = 15;

    // Salidas - Motor
    constexpr uint8_t MOTOR_DIR_LEFT = 12;
    constexpr uint8_t MOTOR_DIR_RIGHT = 13;
    constexpr uint8_t CENTRIFUGE = 14;

    // Salidas - Válvulas
    constexpr uint8_t COLD_WATER_VALVE = 27;
    constexpr uint8_t HOT_WATER_VALVE = 33;
    constexpr uint8_t DRAIN_VALVE = 25;
    constexpr uint8_t DOOR_MAGNET = 26;

    // Comunicación Nextion
    constexpr uint8_t NEXTION_RX = 16;
    constexpr uint8_t NEXTION_TX = 17;

    // Sensores
    constexpr uint8_t PRESSURE_DOUT = 5;
    constexpr uint8_t PRESSURE_SCLK = 4;
    constexpr uint8_t TEMPERATURE = 23;
}

// ========================================
// CONFIGURACIÓN DE SENSORES
// ========================================

namespace SensorConfig
{
    // Temperatura DS18B20
    constexpr uint8_t TEMP_RESOLUTION = 9; // 0.5°C precisión
    constexpr uint8_t TEMP_TOLERANCE = 2;  // ±2°C rango de control
    const uint8_t TEMP_SENSOR_ADDR[8] = {0x28, 0xFF, 0x64, 0x1E, 0x0C, 0x31, 0x18, 0x66};
    // const uint8_t TEMP_SENSOR_ADDR[8] = {0x28, 0xFF, 0x07, 0x03, 0x93, 0x16, 0x04, 0x7A};

    // Sensor de presión HX710B - Calibración MANUAL (sin tare)
    // PASO 1: Con tanque VACÍO, anotar valor pascal() del monitor serial
    // PASO 2: Configurar PRESSURE_OFFSET = valor_vacio
    // PASO 3: Llenar tanque a cada nivel y anotar valores (pascal - offset)
    // PASO 4: Actualizar PRESSURE_LEVEL_1 a PRESSURE_LEVEL_4

    constexpr long PRESSURE_OFFSET = 565; // Valor pascal() cuando tanque vacío (~565 Pa)

    // Niveles de agua (valores después de restar offset)
    constexpr uint16_t PRESSURE_LEVEL_1 = 36; // 601 - 565 = ~36
    constexpr uint16_t PRESSURE_LEVEL_2 = 63; // 628 - 565 = ~63
    constexpr uint16_t PRESSURE_LEVEL_3 = 80; // 645 - 565 = ~80
    constexpr uint16_t PRESSURE_LEVEL_4 = 98; // 663 - 565 = ~98
}

// ========================================
// LÍMITES DE PARÁMETROS
// ========================================

namespace Limits
{
    constexpr uint8_t MIN_WATER_LEVEL = 1;
    constexpr uint8_t MAX_WATER_LEVEL = 4;

    constexpr uint8_t MIN_TEMPERATURE = 5;
    constexpr uint8_t MAX_TEMPERATURE = 100;

    constexpr uint8_t MIN_TIME = 1;
    constexpr uint8_t MAX_TIME = 60;
}

// ========================================
// TIEMPOS DEL SISTEMA
// ========================================

namespace Timing
{
    constexpr uint16_t WELCOME_SCREEN_MS = 1500;      // Tiempo de bienvenida
    constexpr uint16_t DRAIN_TIME_SEC = 45;           // Tiempo de drenaje
    constexpr uint16_t COOLING_TIME_SEC = 60;         // Tiempo de finalizacion
    constexpr uint16_t CENTRIFUGE_TIME_SEC = 180;      // Tiempo de centrifugado
    constexpr uint16_t REST_BETWEEN_PROCESS_SEC = 10; // Tiempo de reposo entre tandas (P24)

    constexpr uint16_t SENSOR_READ_INTERVAL_MS = 500; // Intervalo de lectura de sensores
    constexpr uint16_t UI_UPDATE_INTERVAL_MS = 1000;  // Intervalo de actualización de UI
}

// ========================================
// CONFIGURACIÓN NEXTION
// ========================================

namespace NextionConfig
{
    constexpr uint32_t BAUD_RATE = 115200;

    // Colores Nextion (RGB565)
    constexpr uint16_t COLOR_ACTIVE = 1024;    // Color para botón activo/seleccionado
    constexpr uint16_t COLOR_INACTIVE = 50712; // Color para botón inactivo/no seleccionado
    constexpr uint16_t COLOR_DISABLED = 33840; // Color para botón deshabilitado

    // Páginas
    constexpr uint8_t PAGE_WELCOME = 0;
    constexpr uint8_t PAGE_SELECTION = 1;
    constexpr uint8_t PAGE_EXECUTION = 2;
    constexpr uint8_t PAGE_EDIT = 3;
    constexpr uint8_t PAGE_ERROR = 4;
    constexpr uint8_t PAGE_EMERGENCY = 5;

    // IDs de botones (página selección)
    constexpr uint8_t BTN_PROGRAM1 = 1;
    constexpr uint8_t BTN_PROGRAM2 = 2;
    constexpr uint8_t BTN_PROGRAM3 = 3;
    constexpr uint8_t BTN_START = 22;
    constexpr uint8_t BTN_EDIT = 21;

    // IDs de botones (página ejecución)
    constexpr uint8_t BTN_PAUSE = 21;
    constexpr uint8_t BTN_STOP = 22;

    // IDs de botones (página edición)
    constexpr uint8_t BTN_PARAM_MINUS = 7;
    constexpr uint8_t BTN_PARAM_PLUS = 6;
    constexpr uint8_t BTN_PARAM_PREV = 8;
    constexpr uint8_t BTN_PARAM_NEXT = 5;
    constexpr uint8_t BTN_SAVE = 3;
    constexpr uint8_t BTN_CANCEL = 4;
    constexpr uint8_t BTN_PROCESS1 = 26;
    constexpr uint8_t BTN_PROCESS2 = 27;
    constexpr uint8_t BTN_PROCESS3 = 28;
    constexpr uint8_t BTN_PROCESS4 = 29;

    // IDs de botones del panel derecho (página edición)
    constexpr uint8_t BTN_PANEL_NIVEL = 18;
    constexpr uint8_t BTN_PANEL_TEMP = 19;
    constexpr uint8_t BTN_PANEL_TIEMPO = 20;
    constexpr uint8_t BTN_PANEL_CENTRIF = 23;
    constexpr uint8_t BTN_PANEL_AGUA = 24;
}

// ========================================
// PROGRAMAS Y FASES
// ========================================

enum ProgramType
{
    PROGRAM_22 = 22, // Agua caliente
    PROGRAM_23 = 23, // Agua fría
    PROGRAM_24 = 24  // Multiproceso
};

enum PhaseType
{
    PHASE_FILLING = 0,
    PHASE_WASHING = 1,
    PHASE_DRAINING = 2,
    PHASE_SPINNING = 3,
    PHASE_COOLING = 4
};

enum WaterType
{
    WATER_COLD = 0,
    WATER_HOT = 1
};

#endif // CONFIG_H
