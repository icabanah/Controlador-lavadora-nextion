# Controlador de Lavadora Industrial - ESP32 + Nextion

## 📋 Descripción del Proyecto

Sistema de control para lavadora industrial con 3 programas configurables, usando ESP32 como controlador principal y pantalla Nextion para la interfaz de usuario.

## 🏗️ Arquitectura

### Patrón: Máquina de Estados Simple

```
┌─────────────────────────────────────────────────────┐
│                    main.cpp                         │
│  ┌─────────┐  ┌──────────┐  ┌─────────┐           │
│  │Hardware │  │ Sensors  │  │ Nextion │           │
│  │Control  │  │ Manager  │  │   UI    │           │
│  └─────────┘  └──────────┘  └─────────┘           │
│         │           │             │                 │
│         └───────────┴─────────────┘                 │
│                     │                               │
│              ┌──────▼──────┐                       │
│              │State Machine│                       │
│              └─────────────┘                       │
└─────────────────────────────────────────────────────┘
```

### Estados del Sistema

- `INIT` → `WELCOME` → `SELECTION` → `FILLING` → `WASHING` → `DRAINING` → `SPINNING` → `COOLING` → `COMPLETED`
- Estados especiales: `PAUSED`, `ERROR`, `EMERGENCY`

## 📁 Estructura de Archivos

```
Controlador-lavadora-nextion/
├── platformio.ini          # Configuración PlatformIO
├── CLAUDE.md               # Este archivo
├── include/
│   ├── Config.h            # Configuración central (pines, constantes)
│   ├── StateMachine.h      # Máquina de estados
│   ├── HardwareControl.h   # Control de actuadores
│   ├── SensorManager.h     # Lectura de sensores
│   └── NextionUI.h         # Comunicación Nextion (Serial2 UART)
├── src/
│   ├── main.cpp            # Loop principal + callbacks
│   ├── StateMachine.cpp    # Lógica de fases
│   ├── HardwareControl.cpp # Control hardware
│   ├── SensorManager.cpp   # Gestión sensores
│   └── NextionUI.cpp       # Protocolo Nextion
├── lib/                    # Librerías locales
│   ├── HX710B/             # Sensor de presión (local - no está en registry)
│   └── AsyncTaskLib/       # Tareas asíncronas (OBLIGATORIO USAR)
│
│ Nota: OneWire y DallasTemperature se instalan desde PlatformIO registry
│       (versiones actualizadas y compatibles con ESP32 framework 2.0.16)
└── docs/
    ├── Detalle-programas.md
    ├── config_structs.h
    └── paginas-nextion/    # Capturas de pantalla
```

## ⚙️ Programas Implementados

### Programa 22 - Agua Caliente

- 5 fases: Llenado → Lavado → Drenaje → Centrifugado → Enfriamiento
- Control de temperatura: ±2°C (drena y rellena para ajustar)
- 1 proceso único

### Programa 23 - Agua Fría

- 5 fases iguales a P22
- Sin control de temperatura (solo lectura)
- 1 proceso único

### Programa 24 - Multiproceso

- 4 procesos × 4 fases cada uno + enfriamiento final
- Agua caliente o fría configurable por proceso
- Control de temperatura si es agua caliente

## 🔧 Hardware

### Pines ESP32 (ver [Config.h](include/Config.h))

**Salidas:**

- Motor Izquierda: GPIO 12
- Motor Derecha: GPIO 13
- Centrifugado: GPIO 14
- Válvula Agua Fría: GPIO 27
- Válvula Agua Caliente: GPIO 33
- Válvula Drenaje: GPIO 25
- Electroimán Puerta: GPIO 26

**Entradas:**

- Botón Emergencia: GPIO 15 (INPUT_PULLUP)
- Sensor Temperatura (DS18B20): GPIO 23
- Sensor Presión HX710B DOUT: GPIO 5
- Sensor Presión HX710B SCLK: GPIO 4

**Comunicación Nextion:**

- RX (ESP32 ← Nextion TX): GPIO 16
- TX (ESP32 → Nextion RX): GPIO 17
- Baudrate: 115200

## 📡 Protocolo Nextion

### Comunicación Serial (UART)

**Enviar comandos desde ESP32:**

```cpp
serial->print("page 1");           // Cambiar a página 1
serial->write(0xFF);                // Fin de comando
serial->write(0xFF);
serial->write(0xFF);

// Cambiar texto
serial->print("t0.txt=\"Hola\"");
serial->write(0xFF); serial->write(0xFF); serial->write(0xFF);

// Cambiar número
serial->print("n0.val=25");
serial->write(0xFF); serial->write(0xFF); serial->write(0xFF);
```

**Recibir eventos touch desde Nextion:**

```
Formato: [0x65] [pageId] [componentId] [eventType] [0xFF 0xFF 0xFF]
```

**Configuración en Nextion Editor:**
En cada botón, configurar "Touch Release Event":

```javascript
// Ejemplo: botón "btnPrograma1" en página 1
printh 65 01 01 01
//     │  │  │  └─ eventType (1=release, 0=press)
//     │  │  └──── componentId (ID del botón)
//     │  └─────── pageId (número de página)
//     └────────── Código de evento touch
```

### Mapeo de IDs (ver [Config.h:92-131](include/Config.h))

**Página 1 (Selección):**

- btnPrograma1: ID=1
- btnPrograma2: ID=2
- btnPrograma3: ID=3
- btnComenzar: ID=22
- btnEditar: ID=21

**Página 2 (Ejecución):**

- btnPausar: ID=21
- btnParar: ID=22

## 🚫 REGLAS IMPORTANTES DE CÓDIGO

### ❌ PROHIBIDO: `delay()`

**NUNCA usar `delay()` en el código.** Bloquea el loop principal.

**MAL:**

```cpp
void someFunction() {
    digitalWrite(PIN, HIGH);
    delay(1000);  // ❌ BLOQUEANTE
    digitalWrite(PIN, LOW);
}
```

**BIEN:**

```cpp
#include <AsyncTaskLib.h>

AsyncTask timer(1000, []() {
    digitalWrite(PIN, LOW);
});

void someFunction() {
    digitalWrite(PIN, HIGH);
    timer.Start();
}

void loop() {
    timer.Update();  // Llamar en loop()
}
```

### ✅ Usar AsyncTaskLib

```cpp
// Timer simple (una sola vez)
AsyncTask delayTimer(5000, []() {
    Serial.println("5 segundos pasaron");
});
delayTimer.Start();

// Timer con auto-reset (repetitivo)
AsyncTask periodicTimer(1000, true, []() {
    Serial.println("Cada segundo");
});
periodicTimer.Start();

// En loop()
void loop() {
    delayTimer.Update();
    periodicTimer.Update();
}
```

### Control de Tiempo No Bloqueante

```cpp
// BIEN: Comparación con millis()
unsigned long lastUpdate = 0;
const unsigned long interval = 1000;

void loop() {
    if (millis() - lastUpdate >= interval) {
        lastUpdate = millis();
        // Hacer algo cada 1 segundo
    }
}
```

## 🔍 Debugging

### Monitor Serial (115200 baud)

```cpp
Serial.println("Estado: " + String(currentState));
Serial.print("Temp: "); Serial.println(sensors.getTemperature());
Serial.print("Nivel: "); Serial.println(sensors.getWaterLevel());
```

### Verificar Eventos Nextion

Los eventos se imprimen automáticamente:

```
Nextion Event: Page=1, Comp=1, Type=1
```

## 📊 Calibración de Sensores

### Sensor de Presión HX710B

Ajustar en [Config.h:42-46](include/Config.h):

```cpp
constexpr uint16_t PRESSURE_LEVEL_1 = 601;  // Nivel 1 (bajo)
constexpr uint16_t PRESSURE_LEVEL_2 = 628;  // Nivel 2
constexpr uint16_t PRESSURE_LEVEL_3 = 645;  // Nivel 3
constexpr uint16_t PRESSURE_LEVEL_4 = 663;  // Nivel 4 (alto)
```

**Cómo calibrar:**

1. Leer valor raw: `Serial.println(sensors.getPressureRaw());`
2. Llenar tanque al nivel deseado
3. Anotar valor y actualizar constantes

### Sensor de Temperatura DS18B20

Dirección única del sensor en [Config.h:41](include/Config.h):

```cpp
const uint8_t TEMP_SENSOR_ADDR[8] = {0x28, 0xFF, 0x64, 0x1E, 0x0C, 0x31, 0x18, 0x66};
```

**Cómo obtener dirección:**

```cpp
#include <OneWire.h>
#include <DallasTemperature.h>

OneWire oneWire(23);
DallasTemperature sensors(&oneWire);

void setup() {
    Serial.begin(115200);
    sensors.begin();

    uint8_t addr[8];
    if (oneWire.search(addr)) {
        Serial.print("Dirección: {");
        for (int i = 0; i < 8; i++) {
            Serial.print("0x");
            Serial.print(addr[i], HEX);
            if (i < 7) Serial.print(", ");
        }
        Serial.println("}");
    }
}
```

## 🛠️ Compilación y Carga

```bash
# Compilar
pio run

# Cargar al ESP32
pio run --target upload

# Monitor serial
pio device monitor
```

Desde VSCode:

- Compilar: `Ctrl+Alt+B` (PlatformIO: Build)
- Cargar: `Ctrl+Alt+U` (PlatformIO: Upload)
- Monitor: `Ctrl+Alt+S` (PlatformIO: Serial Monitor)

## 🔍 Debugging

### Logging Estructurado con esp_log

El proyecto usa el sistema de logging nativo de ESP32 que respeta `CORE_DEBUG_LEVEL` en [platformio.ini](platformio.ini).

**Niveles de log** (configurar en platformio.ini):
```ini
build_flags =
    -D CORE_DEBUG_LEVEL=3  ; 0=None, 1=Error, 2=Warn, 3=Info, 4=Debug, 5=Verbose
```

**Uso en código** (ver [Debug.h](include/Debug.h)):
```cpp
#include "Debug.h"

// En cualquier archivo .cpp
log_v(TAG_SENSOR, "Mensaje muy detallado");    // Verbose
log_d(TAG_SENSOR, "Info de debug");             // Debug
log_i(TAG_SENSOR, "Información importante");    // Info
log_w(TAG_SENSOR, "Advertencia");               // Warning
log_e(TAG_SENSOR, "Error crítico");             // Error
```

**Tags disponibles:**
- `TAG_MAIN` - Loop principal
- `TAG_STATE` - Máquina de estados
- `TAG_HARDWARE` - Control de hardware
- `TAG_SENSOR` - Lectura de sensores
- `TAG_NEXTION` - Comunicación Nextion

**Macros útiles:**
```cpp
DEBUG_STATE_TRANSITION(from, to);
DEBUG_SENSOR_READ("Temperatura", 25.5);
DEBUG_HARDWARE_ACTION("Motor izquierda ON");
DEBUG_NEXTION_EVENT(page, comp, type);
```

### Monitor Serial Mejorado

El monitor serial tiene **filtros de color y timestamps**:
```bash
# Monitor con colores y timestamp
pio device monitor

# Desde VSCode: Ctrl+Alt+S
```

Salida ejemplo:
```
[00:05:23.456] I (1234) SENSOR: Temperatura = 25.30
[00:05:23.512] D (1235) STATE: Transición: 2 -> 3
[00:05:23.678] W (1240) HARDWARE: Nivel de agua bajo
```

### Unit Testing

Ejecutar tests unitarios **sin hardware**:

```bash
# Correr todos los tests
pio test

# Correr test específico
pio test -f test_state_machine

# Correr tests en el ESP32 (requiere hardware)
pio test --environment esp32dev
```

**Crear nuevos tests:**
1. Crear archivo en `test/test_nombre.cpp`
2. Usar framework Unity:
```cpp
#include <unity.h>

void test_mi_funcion() {
    TEST_ASSERT_EQUAL(expected, actual);
    TEST_ASSERT_TRUE(condition);
    TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual);
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_mi_funcion);
    UNITY_END();
}

void loop() {}
```

Ver ejemplo completo en [test/test_state_machine.cpp](test/test_state_machine.cpp)

### Utilidades de Debug

**Monitorear memoria:**
```cpp
#include "Debug.h"

DebugUtils::printMemoryInfo();  // Muestra heap libre
DebugUtils::printChipInfo();    // Info del ESP32
```

**Monitor de performance:**
```cpp
DebugUtils::LoopMonitor monitor;

void setup() {
    monitor.start();
}

void loop() {
    monitor.update();  // Muestra stats cada 10 seg
}
```

**Simulador de sensores** (testing sin hardware):
```cpp
#include "Debug.h"

float temp = DebugUtils::SensorSimulator::getSimulatedTemperature();
uint8_t level = DebugUtils::SensorSimulator::getSimulatedWaterLevel();
```

### Cambiar Nivel de Log en Runtime

```cpp
#include "Debug.h"

void setup() {
    // Cambiar nivel dinámicamente
    DebugUtils::setLogLevel(ESP_LOG_DEBUG);  // Nivel 4
}
```

## 🐛 Solución de Problemas

### Error de compilación con librerías

**Configuración actual (compatible con ESP32 framework 2.0.16):**
- Platform: **espressif32@6.7.0**
- OneWire y DallasTemperature: **Desde PlatformIO registry** (actualizadas)
- HX710B y AsyncTaskLib: **Locales en lib/** (no están en registry)
- Ver [platformio.ini:21-23](platformio.ini#L21-23)

**Si aparece error `esp32_gpioMux was not declared`:**
- Las versiones locales de OneWire en `lib/` están deshabilitadas (`.bak`)
- PlatformIO descargará automáticamente versiones compatibles

### Error: "HX710B.h not found"

- Verificar que `lib/HX710B/HX710B.h` existe
- Ejecutar: `pio run --target clean` y recompilar

### Nextion no responde

1. Verificar conexiones: TX↔RX cruzados
2. Verificar baudrate: 115200 en ambos lados
3. Probar comando manual: `sendCommand("page 0");`

### Sensor de temperatura retorna -127°C

- Sensor desconectado o dirección incorrecta
- Verificar pull-up de 4.7kΩ en línea de datos

### Motor no alterna dirección

- Verificar `HardwareControl::toggleMotorDirection()` se llama en loop
- Ajustar `MOTOR_TOGGLE_INTERVAL_MS` en [HardwareControl.h](include/HardwareControl.h)

## 📋 Página de Selección

### Resaltado de Botones de Programa

Los botones de programa (P22, P23, P24) cambian de color para indicar cuál está seleccionado.

**Función `updateProgramButtons()` ([main.cpp:49-72](src/main.cpp)):**

```cpp
void updateProgramButtons(uint8_t selectedProgram) {
    for (int i = 0; i < 3; i++) {
        char btnName[16];
        snprintf(btnName, sizeof(btnName), "btnPrograma%d", i + 1);

        if (programNumbers[i] == selectedProgram) {
            nextion.setBackgroundColor(btnName, COLOR_ACTIVE);    // Verde
        } else {
            nextion.setBackgroundColor(btnName, COLOR_INACTIVE);  // Gris claro
        }
    }
}
```

**Cuándo se actualiza:**
- Al presionar botón de programa (líneas 296, 306, 316)
- Al entrar a página de selección (línea 499)
- Al salir de página de edición (líneas 450, 462)

**Resultado Visual:**
- Programa seleccionado: Color verde (COLOR_ACTIVE = 1024)
- Programas no seleccionados: Color gris claro (COLOR_INACTIVE = 50712)

## 🎛️ Página de Edición

### Funcionalidad Implementada

La página de edición permite modificar parámetros de cada proceso/tanda:

**Selección de Tanda:**
- P22/P23: Solo tanda1 disponible (tandas 2-4 deshabilitadas)
- P24: 4 tandas disponibles

**Parámetros Editables:**
- Nivel de agua (1-4)
- Temperatura (°C)
- Tiempo (minutos)
- Centrifugado (Sí/No)
- Tipo de agua (Caliente/Fría)

**Navegación:**
1. Seleccionar tanda con botones `tanda1`-`tanda4` (P22/P23: solo tanda1; P24: todas)
2. Seleccionar parámetro:
   - Presionando botón del panel derecho (`val_nivel`, `val_temp`, etc.)
   - O usando botones "Siguiente"/"Anterior" (BTN_PARAM_NEXT/PREV - navegación cíclica)
3. Modificar valor con botones `+` / `-`
4. Guardar: Primera pulsación guarda, segunda vuelve a selección
5. Cancelar: Descarta cambios y vuelve a selección

**Implementación en [main.cpp:28-221](src/main.cpp):**

```cpp
// Estructura de estado de edición
struct EditState {
    uint8_t currentTanda;        // 0-3
    ParameterType currentParam;  // Parámetro seleccionado
    bool editingValue;           // Flag de guardado
    ProgramConfig backupConfig;  // Backup para cancelar
} editState;

// Funciones principales
void enterEditMode()               // Inicializa modo edición con backup
void updateEditDisplay()           // Actualiza todos los componentes Nextion
void incrementCurrentParameter()   // Incrementa parámetro actual
void decrementCurrentParameter()   // Decrementa parámetro actual
void nextParameter()               // Navega al siguiente parámetro (cíclico)
void prevParameter()               // Navega al parámetro anterior (cíclico)
```

**Validación de Tandas:**
- Botones `tanda2`, `tanda3`, `tanda4` solo responden en P24 (4 procesos)
- En P22/P23 (1 proceso): botones se deshabilitan con `.en=0` (no presionables)
- Estado visual: `.val=1` (seleccionado/resaltado), `.val=0` (disponible/normal)
- Cambio de tanda actualiza automáticamente todos los valores del panel

**Control de Estado de Botones ([main.cpp:89-111](src/main.cpp)):**
```cpp
// Para cada tanda
if (i < totalTandas) {
    nextion.setEnabledById(tandaIds[i], true);  // Habilitado con comando tsw

    if (i == tanda) {
        nextion.setBackgroundColor(tandaName, COLOR_ACTIVE);    // Color activo
        nextion.setNumber(tandaName, 1);  // Valor seleccionado
    } else {
        nextion.setBackgroundColor(tandaName, COLOR_INACTIVE);  // Color inactivo
        nextion.setNumber(tandaName, 0);  // Valor no seleccionado
    }
} else {
    nextion.setEnabledById(tandaIds[i], false);  // Deshabilitado con tsw
    nextion.setBackgroundColor(tandaName, COLOR_DISABLED);  // Color deshabilitado
    nextion.setNumber(tandaName, 0);
}
```

**Comandos Nextion:**
- `tsw 26,1` → Habilita botón con ID 26 (tanda1)
- `tsw 27,0` → Deshabilita botón con ID 27 (tanda2)
- `tanda1.bco=1024` → Cambia color de fondo a COLOR_ACTIVE
- `tanda2.bco=50712` → Cambia color de fondo a COLOR_INACTIVE
- `tanda3.bco=33840` → Cambia color de fondo a COLOR_DISABLED

**Colores Definidos ([Config.h:88-91](include/Config.h)):**
- `COLOR_ACTIVE = 1024` (Verde/Activo - botón seleccionado)
- `COLOR_INACTIVE = 50712` (Gris claro - botón disponible)
- `COLOR_DISABLED = 33840` (Gris oscuro - botón deshabilitado)

**IDs de Componentes del Panel ([Config.h:119-124](include/Config.h)):**
- `BTN_PANEL_NIVEL = 18`
- `BTN_PANEL_TEMP = 19`
- `BTN_PANEL_TIEMPO = 20`
- `BTN_PANEL_CENTRIF = 23`
- `BTN_PANEL_AGUA = 24`

## 💾 Almacenamiento Persistente

### Módulo Storage - Preferences ESP32

El sistema guarda las configuraciones de los 3 programas en la memoria flash del ESP32 usando la biblioteca Preferences.

**Características:**
- Almacenamiento persistente (datos permanecen después de apagar el ESP32)
- Restauración automática de valores de fábrica en primer arranque
- Namespace: `"washer"`

**Estructura de Claves en Preferences:**
```
Formato: p{programNumber}_{processIndex}_{parámetro}

Ejemplos:
- p22_0_nivel  → Nivel de agua del proceso 0 del programa 22
- p22_0_temp   → Temperatura del proceso 0 del programa 22
- p24_2_centrif → Centrifugado del proceso 2 del programa 24
- p24_total    → Número total de procesos del programa 24
```

**Funciones Principales ([Storage.h](include/Storage.h), [Storage.cpp](src/Storage.cpp)):**

```cpp
// Inicialización (en setup)
storage.begin();

// Guardar programa completo
storage.saveProgram(programNumber, config);

// Cargar programa completo
storage.loadProgram(programNumber, config);

// Guardar solo un proceso específico
storage.saveProcess(programNumber, processIndex, config);

// Restaurar valores de fábrica (todos los programas)
storage.restoreDefaults();

// Verificar si existe configuración guardada
storage.hasStoredConfig(programNumber);

// Limpiar toda la memoria
storage.clearAll();
```

**Flujo de Uso:**

1. **Al iniciar el ESP32:**
   - `storage.begin()` verifica si es primera vez
   - Si es primera vez: crea valores de fábrica para P22, P23, P24
   - Si ya hay datos: los deja intactos

2. **Al seleccionar un programa:**
   - Intenta cargar configuración guardada: `storage.loadProgram()`
   - Si no existe: usa valores por defecto

3. **Al editar parámetros:**
   - Usuario modifica valores en página de edición
   - Primer clic en "Guardar": `storage.saveProgram()` guarda en flash
   - Segundo clic en "Guardar": vuelve a página de selección

4. **Al cancelar edición:**
   - Restaura backup temporal (no afecta memoria persistente)

**Implementación en [main.cpp](src/main.cpp):**

```cpp
// Inicialización (línea 542-543)
Serial.println("Inicializando almacenamiento...");
storage.begin();

// Cargar al seleccionar programa (líneas 263-266)
if (!storage.loadProgram(22, stateMachine.getConfig())) {
    stateMachine.getConfig().setDefaults(PROGRAM_22);
}

// Guardar al editar (línea 411)
storage.saveProgram(config.programNumber, config);
```

**Valores Guardados por Proceso:**
- `nivel` (uint8_t): Nivel de agua (1-4)
- `temp` (uint8_t): Temperatura (°C)
- `time` (uint8_t): Tiempo (minutos)
- `centrif` (bool): Centrifugado habilitado
- `water` (uint8_t): Tipo de agua (0=Fría, 1=Caliente)

**Inicialización al Cargar ([Storage.cpp:85-88](src/Storage.cpp)):**
```cpp
// Al cargar, se inicializan automáticamente:
config.programNumber = programNumber;
config.currentProcess = 0;
config.currentPhase = PHASE_FILLING;
```

**Carga al Inicio ([main.cpp:594-600](src/main.cpp)):**
```cpp
// Después de stateMachine.begin(), cargar configuración guardada
if (storage.loadProgram(22, stateMachine.getConfig())) {
    Serial.println("Configuración de P22 cargada desde memoria");
} else {
    Serial.println("Usando configuración por defecto de P22");
}
```

**Límites de Memoria:**
- Preferences usa partición NVS del ESP32 (típicamente 20KB)
- Cada programa ocupa ~40 bytes (3 programas × 4 procesos × 5 parámetros)
- Espacio total usado: ~120 bytes
- Amplio margen disponible para futuras expansiones

**IMPORTANTE - Persistencia de Datos:**
✅ Los valores editados se guardan en flash al presionar "Guardar" en página de edición
✅ Al reiniciar el ESP32, se cargan automáticamente las configuraciones guardadas
✅ Si no hay configuración guardada, se usan valores de fábrica
✅ La configuración persiste incluso después de desconectar la alimentación

## 📝 TODOs Pendientes

- [x] Implementar navegación completa en página de edición
- [x] Guardar configuraciones en EEPROM/NVS
- [ ] Agregar watchdog timer para emergencias
- [ ] Crear archivo .HMI para Nextion Editor
- [ ] Pruebas con hardware real

## 📚 Referencias

- [Protocolo Nextion](https://nextion.tech/instruction-set/)
- [HX710B Datasheet](https://github.com/kurimawxx00/hx710B_pressure_sensor)
- [DS18B20 Datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/DS18B20.pdf)
- [AsyncTaskLib](https://github.com/luisllamasbinaburo/Arduino-AsyncTask)
- [PlatformIO Docs](https://docs.platformio.org/)

---

**Última actualización:** 2025-10-05
**Autor:** Claude + Daniele
**Versión:** 1.0.0
