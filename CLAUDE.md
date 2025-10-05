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
├── lib/                    # Librerías locales (ya incluidas)
│   ├── HX710B/             # Sensor de presión
│   ├── DallasTemperature/  # Sensor DS18B20
│   ├── OneWire/            # Protocolo OneWire
│   ├── AsyncTaskLib/       # Tareas asíncronas (OBLIGATORIO USAR)
│   └── Adafruit_BusIO/     # Dependencia I2C/SPI
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

## 🐛 Solución de Problemas

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

## 📝 TODOs Pendientes

- [ ] Implementar navegación completa en página de edición ([main.cpp:104-111](src/main.cpp))
- [ ] Guardar configuraciones en EEPROM/NVS
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
