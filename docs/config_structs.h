#ifndef CONFIG_STRUCTS_H
#define CONFIG_STRUCTS_H

#include "Arduino.h"

// === ESTRUCTURAS ORGANIZADAS PARA CONFIGURACIÓN ===

// 📌 CONFIGURACIÓN DE PINES HARDWARE
struct HardwarePins
{
  // Entrada de emergencia
  static constexpr uint8_t EMERGENCY_BUTTON = 15; // pin botón físico de emergencia

  // Salidas - Motor
  static constexpr uint8_t MOTOR_DIR_LEFT = 12;  // pin activa dirección izquierda
  static constexpr uint8_t MOTOR_DIR_RIGHT = 13; // pin activa dirección derecha
  static constexpr uint8_t CENTRIFUGE = 14;      // pin activa centrifugado

  // Salidas - Válvulas
  static constexpr uint8_t COLD_WATER_VALVE = 27; // Válvula para agua fría
  static constexpr uint8_t HOT_WATER_VALVE = 33;  // Válvula para agua caliente
  static constexpr uint8_t DRAIN_VALVE = 25;      // Válvula para desfogue de agua
  static constexpr uint8_t DOOR_MAGNET = 26;      // Electroimán para bloqueo de puerta

  // Comunicación Nextion
  static constexpr uint8_t NEXTION_RX = 16; // RX de Nextion (a TX del Nextion)
  static constexpr uint8_t NEXTION_TX = 17; // TX de Nextion (a RX del Nextion)

  // Sensores
  static constexpr uint8_t PRESSURE_DOUT = 5; // Pin Data del sensor de presión
  static constexpr uint8_t PRESSURE_SCLK = 4; // Pin Clock del sensor de presión
  static constexpr uint8_t TEMPERATURE = 23;  // Pin para sensor de temperatura DS18B20
};

// 🌡️ CONFIGURACIÓN DE SENSORES
struct SensorConfig
{
  // Temperatura
  static constexpr uint8_t TEMP_RESOLUTION = 9;                                                       // 9-bit resolución del sennsor de temperatura (0.5°C)
  static constexpr uint8_t TEMP_RANGE = 2;                                                            // Rango de temperatura antes de corregir
  static inline const uint8_t TEMP_SENSOR_ADDR[8] = {0x28, 0xFF, 0x64, 0x1E, 0x0C, 0x31, 0x18, 0x66}; // Dirección fija del sensor DS18B20

  // Presión (niveles de agua)
  static constexpr uint16_t PRESSURE_LEVEL_1 = 601; // Valor analógico para nivel 1
  static constexpr uint16_t PRESSURE_LEVEL_2 = 628; // Valor analógico para nivel 2
  static constexpr uint16_t PRESSURE_LEVEL_3 = 645; // Valor analógico para nivel 3
  static constexpr uint16_t PRESSURE_LEVEL_4 = 663; // Valor analógico para nivel 4
};

// 📐 LÍMITES DE PARÁMETROS
struct ParameterLimits
{
  // Nivel de agua
  static constexpr uint8_t MIN_LEVEL = 1; // Mínimo nivel de agua
  static constexpr uint8_t MAX_LEVEL = 4; // Máximo nivel de agua

  // Temperatura
  static constexpr uint8_t MIN_TEMPERATURE = 5;   // Mínima temperatura (°C)
  static constexpr uint8_t MAX_TEMPERATURE = 100; // Máxima temperatura (°C)

  // Tiempo
  static constexpr uint8_t MIN_TIME = 1;  // Mínimo tiempo (minutos)
  static constexpr uint8_t MAX_TIME = 60; // Máximo tiempo (minutos)
};

// ⏱️ TIEMPOS DEL SISTEMA
struct SystemTiming
{
  static constexpr uint16_t WELCOME_SCREEN = 3000; // ms pantalla de bienvenida
  static constexpr uint16_t DRAIN_TIME = 45;       // segundos
  static constexpr uint16_t DOOR_LOCK_TIME = 60;   // segundos
  static constexpr uint16_t CENTRIFUGE_TIME = 45;  // segundos
  static constexpr uint8_t NEXTION_END_CMD[3] = {0xFF, 0xFF, 0xFF};
};

// 🔧 CONFIGURACIÓN DE PROGRAMAS
struct ProgramDefaults
{
  // Temperaturas por defecto [P22, P23]
  static constexpr uint8_t TEMPERATURES_22_23[2] = {25, 25};

  // Temperaturas por defecto [P24]
  static constexpr uint8_t TEMPERATURES_24[4] = {25, 25, 25, 25};

  // Tiempos por defecto [P22, P23]
  static constexpr uint8_t TIMES_22_23[2] = {15, 15};

  // Tiempos por defecto [P24]
  static constexpr uint8_t TIMES_24[4] = {15, 15, 15, 15};

  // Niveles de agua por defecto [P22, P23]
  static constexpr uint8_t WATER_LEVELS_22_23[2] = {3, 3};

  // Niveles de agua por defecto [P24]
  static constexpr uint8_t WATER_LEVELS_24[4] = {3, 3, 3, 3};
};

// 🖥️ NEXTION DISPLAY
struct NextionConfig
{
  static constexpr uint32_t BAUD_RATE = 115200;
  static constexpr uint8_t TIMEOUT = 5;

  // Páginas
  static constexpr uint8_t PAGE_WELCOME = 0;
  static constexpr uint8_t PAGE_SELECTION = 1;
  static constexpr uint8_t PAGE_EXECUTION = 2;
  static constexpr uint8_t PAGE_EDIT = 3;
  static constexpr uint8_t PAGE_ERROR = 4;
  static constexpr uint8_t PAGE_EMERGENCY = 5;
};

#endif // CONFIG_STRUCTS_H