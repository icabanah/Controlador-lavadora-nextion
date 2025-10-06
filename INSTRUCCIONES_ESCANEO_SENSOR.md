# Instrucciones para Escanear Sensor DS18B20

## Opción 1: Usar el programa de escaneo dedicado

### Paso 1: Modificar src temporalmente

1. **Renombra** el archivo `src/main.cpp` a `src/main.cpp.bak`
2. **Copia** el archivo `scan_temp_sensor.cpp` a `src/main.cpp`

```bash
# En terminal (Git Bash o PowerShell):
cd src
mv main.cpp main.cpp.bak
cp ../scan_temp_sensor.cpp main.cpp
```

### Paso 2: Compilar y cargar

En VSCode:
- Presiona `Ctrl+Alt+U` para compilar y cargar

O en terminal:
```bash
pio run --target upload
```

### Paso 3: Ver resultado en monitor serial

En VSCode:
- Presiona `Ctrl+Alt+S` para abrir monitor serial

O en terminal:
```bash
pio device monitor
```

### Paso 4: Copiar dirección

Verás algo como:
```
========================================
Escaneando sensores DS18B20...
========================================

Dispositivos encontrados: 1

Sensor #1:
  Dirección: {0x28, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11}
  CRC: OK
  Temperatura actual: 23.5 °C

========================================
Escaneo completado.
Copia la dirección mostrada arriba a Config.h
========================================
```

**COPIA LA DIRECCIÓN** mostrada (ejemplo: `{0x28, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11}`)

### Paso 5: Actualizar Config.h

1. Abre `include/Config.h`
2. Busca la línea 41 (aproximadamente):
   ```cpp
   const uint8_t TEMP_SENSOR_ADDR[8] = {0x28, 0xFF, 0x64, 0x1E, 0x0C, 0x31, 0x18, 0x66};
   ```
3. Reemplaza con la dirección que copiaste
4. Guarda el archivo

### Paso 6: Restaurar main.cpp original

```bash
cd src
rm main.cpp
mv main.cpp.bak main.cpp
```

### Paso 7: Compilar y cargar programa principal

```bash
pio run --target upload
```

---

## Opción 2: Agregar comando de escaneo al programa principal

Si prefieres, puedo agregar un comando de escaneo que se ejecute automáticamente en el `setup()` del programa principal cuando presionas un botón específico.

---

## Verificación de Conexiones

Si NO se encuentran sensores, verifica:

1. **Alimentación:**
   - VCC del sensor → 3.3V o 5V del ESP32
   - GND del sensor → GND del ESP32

2. **Datos:**
   - DATA del sensor → GPIO 23 del ESP32

3. **Resistor pull-up:**
   - 4.7kΩ entre DATA y VCC
   - **MUY IMPORTANTE:** Sin este resistor, el sensor NO funcionará

4. **Cable:**
   - Máximo 5-10 metros para conexiones confiables
   - Cable apantallado si hay ruido eléctrico

---

## Formato de Dirección para Config.h

La dirección siempre tiene **8 bytes** en hexadecimal:

```cpp
const uint8_t TEMP_SENSOR_ADDR[8] = {
    0x28,  // Byte 0: Código de familia (siempre 0x28 para DS18B20)
    0xXX,  // Byte 1-6: Número de serie único
    0xXX,
    0xXX,
    0xXX,
    0xXX,
    0xXX,
    0xXX   // Byte 7: CRC
};
```

---

**NOTA:** Después de actualizar Config.h, el programa principal usará automáticamente el nuevo sensor.
