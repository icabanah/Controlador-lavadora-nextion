# Detalle de Programas y Configuraciones

## Tabla de Configuración de Programas

| Programa | Descripción    | Fases | Procesos | Tipo de Agua    | Control de Temp. | Nivel de Agua | Rotación     | Velocidad    | Centrifugado |
|----------|-------------   |-------|--------  |--------------   |------------------|---------------|----------    |-----------   |--------------|
| 22       | Agua Caliente  | 5     | 1        | Caliente (fijo) | Sí               | Configurable  | Fijo         | Fijo         | Configurable |
| 23       | Agua Fría      | 5     | 1        | Fría (fijo)     | No               | Configurable  | Fijo         | Fijo         | Configurable |
| 24       | Multiproceso   | 16    | 4        | Fría/Caliente   | Sí/No            | Configurable  | Fijo         | Fijo         | Configurable |

## Detalle de Programas y Fases

### Programa 22 (Agua Caliente)

- **Tipo de Agua**: Solo agua caliente (no configurable)
- **Control de Temperatura**:
  - Mantiene temperatura en rango objetivo ± 2°C
  - Si temperatura > objetivo + 2°C: drena agua parcialmente y rellena con agua fría.
  - Si temperatura < objetivo - 2°C: drena agua parcialmente y rellena con agua caliente.
- **Comentarios**:
  - En la pagina de ejecucion el temporizador no comienza a contar hasta que el nivel del agua llegue al nivel seteado. Es decir, a partir de la fase de lavado. 
  - Drenaje se cierra al comienzo del programa y solo se abre al inicio de la fase "FASE_DRENAJE" y permanece abierto hasta que se inicie otro programa.
  - La puerta se cierra al comienzo del programa y solo se abre al final de la fase "FASE_ENFRIAMIENTO" y permanece abierto hasta que se inicie otro programa.
- **Fases**:
  - Fase 0: FASE_LLENADO (Llena agua hasta que el nivel del agua llegue al seteado)
  - Fase 1: FASE_LAVADO (La lavadora activa permutando los pines izquierda y derecha)
  - Fase 2: FASE_DRENAJE (Activa drenaje de agua de lavadora)
  - Fase 3: FASE_CENTRIFUGA (Si se activa, drenaje permanece abierto y puerta permanece cerrado, pines de izquierda y derecha permanecen desactivados)
  - Fase 4: FASE_ENFRIAMIENTO (Para estabilizar el agua y discurrimiento de agua, la puerta se abre al final).

### Programa 23 (Agua Fría)

- **Tipo de Agua**: Solo agua fría (no configurable)
- **Control de Temperatura**: No (sensor solo informativo)
- **Comentarios**:
  - En la pagina de ejecucion el temporizador no comienza a contar hasta que el nivel del agua llegue al nivel seteado. Es decir, a partir de la fase de lavado. 
  - Drenaje se cierra al comienzo del programa y solo se abre al inicio de la fase "FASE_DRENAJE" y permanece abierto hasta que se inicie otro programa.
  - La puerta se cierra al comienzo del programa y solo se abre al final de la fase "FASE_ENFRIAMIENTO" y permanece abierto hasta que se inicie otro programa.
- **Fases**:
  - Fase 0: FASE_LLENADO (Llena agua hasta que el nivel del agua llegue al seteado)
  - Fase 1: FASE_LAVADO (La lavadora activa permutando los pines izquierda y derecha)
  - Fase 2: FASE_DRENAJE (Activa drenaje de agua de lavadora)
  - Fase 3: FASE_CENTRIFUGA (Si se activa, drenaje permanece abierto y puerta permanece cerrado, pines de izquierda y derecha permanecen desactivados)
  - Fase 4: FASE_ENFRIAMIENTO (Para estabilizar el agua y discurrimiento de agua, la puerta se abre al final).

### Programa 24 (Multiproceso)

- **Tipo de Agua**: Configurable (Fría/Caliente)
- **Control de Temperatura**:
  - Sí, agua caliente: igual que Programa 22
  - No, agua fría: sin control (como Programa 23)
- **Comentarios**:
  - En la pagina de ejecucion el temporizador no comienza a contar hasta que el nivel del agua llegue al nivel seteado. Es decir, a partir de la fase de lavado. 
  - El programa 24 consta de 4 procesos. Y cada proceso de sus fases.
  - Drenaje se cierra al comienzo del programa y solo se abre al inicio de la fase "FASE_DRENAJE" y permanece abierto hasta que se inicie otro proceso. Al inicio de la fase de llenado de cada proceso.
  - La puerta se cierra al comienzo del programa y solo se abre al final de la fase "FASE_ENFRIAMIENTO" y permanece abierto hasta que se inicie otro programa. Es decir, se abre al terminar el programa 24, nunca entre fases.
- **Estructura**:
  - 4 procesos (Cada una con sus fases detallados abajo)
  **Fases**:
  - Proceso 1:
    - Fase 0: FASE_LLENADO (Llena agua hasta que el nivel del agua llegue al seteado)
    - Fase 1: FASE_LAVADO (La lavadora activa permutando los pines izquierda y derecha)
    - Fase 2: FASE_DRENAJE (Activa drenaje de agua de lavadora)
    - Fase 3: FASE_CENTRIFUGA (Si se activa, drenaje permanece abierto y puerta permanece cerrado, pines de izquierda y derecha permanecen desactivados)
  - Proceso 2:
    - Fase 0: FASE_LLENADO (Llena agua hasta que el nivel del agua llegue al seteado)
    - Fase 1: FASE_LAVADO (La lavadora activa permutando los pines izquierda y derecha)
    - Fase 2: FASE_DRENAJE (Activa drenaje de agua de lavadora)
    - Fase 3: FASE_CENTRIFUGA (Si se activa, drenaje permanece abierto y puerta permanece cerrado, pines de izquierda y derecha permanecen desactivados)
  - Proceso 3:
    - Fase 0: FASE_LLENADO (Llena agua hasta que el nivel del agua llegue al seteado)
    - Fase 1: FASE_LAVADO (La lavadora activa permutando los pines izquierda y derecha)
    - Fase 2: FASE_DRENAJE (Activa drenaje de agua de lavadora)
    - Fase 3: FASE_CENTRIFUGA (Si se activa, drenaje permanece abierto y puerta permanece cerrado, pines de izquierda y derecha permanecen desactivados)
  - Proceso 4:
    - Fase 0: FASE_LLENADO (Llena agua hasta que el nivel del agua llegue al seteado)
    - Fase 1: FASE_LAVADO (La lavadora activa permutando los pines izquierda y derecha)
    - Fase 2: FASE_DRENAJE (Activa drenaje de agua de lavadora)
    - Fase 3: FASE_CENTRIFUGA (Si se activa, drenaje permanece abierto y puerta permanece cerrado, pines de izquierda y derecha permanecen desactivados)
  - Fase final: FASE_ENFRIAMIENTO (Para estabilizar el agua y discurrimiento de agua, la puerta se abre al final).

## Configuracion para comunicación con pantalla nextion

### PAGINAS DE LA PANTALLA NEXTION

- NEXTION_PAGE_WELCOME page0    // Página de bienvenida
- NEXTION_PAGE_SELECTION page1  // Página de selección de programa
- NEXTION_PAGE_EXECUTION page2  // Página de ejecución
- NEXTION_PAGE_EDIT page3       // Página de edición de parámetros
- NEXTION_PAGE_ERROR page4      // Página de error
- NEXTION_PAGE_EMERGENCY page5  // Página de emergencia

### Componentes de la pantalla de bienvenida (NEXTION_PAGE_WELCOME)

Componentes editables desde esp32 en la página de bienvenida

- NEXTION_COMP_TITULO "lbl_titulo"        // objName en nextion del título del sistema
- NEXTION_COMP_SUBTITULO "lbl_subtitulo"  // objName en nextion del subtítulo del sistema
- NEXTION_COMP_CONTACTO "lbl_contacto"    // objName en nextion del contacto del sistema

### Componentes de la pantalla seleccion (NEXTION_PAGE_SELECTION)

Componentes editables desde esp32 en la página de selección:

- NEXTION_COMP_SET_PROG "progr_sel"       // objName en nextion del programa seleccionado
- NEXTION_COMP_SET_NIVEL "val_nivel"      // objName en nextion del nivel de agua del programa seleccionado
- NEXTION_COMP_SET_TEMP "val_temp"        // objName en nextion de la temperatura del programa seleccionado
- NEXTION_COMP_SET_TIEMPO "val_tiempo"    // objName en nextion del tiempo de ejecución del programa seleccionado
- NEXTION_COMP_SET_FASE "val_fase"        // objName en nextion de la fase actual (llenado, lavado, Drenaje, Centrifugado (opcional), enfriamiento) del programa seleccionado
- NEXTION_COMP_SET_CENTRIF "val_centrif"  // objName en nextion del centrifugado (Sí/No) del programa seleccionado
- NEXTION_COMP_SET_AGUA "val_agua"        // objName en nextion del tipo de agua (caliente o fria) del programa seleccionado
- NEXTION_COMP_MSG "mensaje"              // objName en nextion del área de texto para mensajes o notificaciones

Botones:

- NEXTION_COMP_BTN_PROGRAM1 "btnPrograma1" // objName en nextion del botón de programa 1
- NEXTION_COMP_BTN_PROGRAM2 "btnPrograma2" // objName en nextion del botón de programa 2
- NEXTION_COMP_BTN_PROGRAM3 "btnPrograma3" // objName en nextion del botón de programa 3
- NEXTION_COMP_BTN_START    "btnComenzar"  // objName en nextion del botón de comenzar
- NEXTION_COMP_BTN_EDIT     "btnEditar"    // objName en nextion del botón de editar

- NEXTION_ID_BTN_PROGRAM1 1 // ID en nextion del botón de programa 1
- NEXTION_ID_BTN_PROGRAM2 2 // ID en nextion del botón de programa 2
- NEXTION_ID_BTN_PROGRAM3 3 // ID en nextion del botón de programa 3
- NEXTION_ID_BTN_START 22   // ID en nextion del botón de comenzar
- NEXTION_ID_BTN_EDIT 21    // ID en nextion del botón de editar

### Componentes de la pantalla ejecucion (NEXTION_PAGE_EXECUTION)

- NEXTION_COMP_PROG_EJECUCION "progr_ejec"            // objName en nextion del programa actual en ejecución
- NEXTION_COMP_FASE_EJECUCION "fase_ejec"             // objName en nextion de la fase actual en ejecución
- NEXTION_COMP_TANDA_EJECUCION "tanda_ejec"           // objName en nextion de la tanda (proceso en caso de multiproceso) actual en ejecución
- NEXTION_COMP_TIEMPO_EJECUCION "tiempo_ejec"         // objName en nextion del tiempo de proceso actual en ejecución transcurrido (00:00) minutos:segundos
- NEXTION_COMP_TIEMPO_TOTAL_EJECUCION "tiempo_total"  // objName en nextion del tiempo total de ejecución

- NEXTION_COMP_TEMP_EJECUCION "temp_ejec"             // objName en nextion de la temperatura en ejecución
- NEXTION_COMP_NIVEL_EJECUCION "nivel_ejec"           // objName en nextion del nivel de agua del proceso actual en ejecución
- NEXTION_COMP_BARRA_TEMP_EJECUCION "barra_temp"      // objName en nextion de la barra del tiempo transcurrido (0 a 100)
- NEXTION_COMP_BARRA_NIVEL_EJECUCION "barra_nivel"    // objName en nextion de la barra del nivel de agua (0 a 100 = 0 a 4)
<!-- - NEXTION_COMP_BARRA_VELOC_EJECUCION "barra_veloc"    // objName en nextion de la barra de velocidad (0 a 100) -->

- NEXTION_COMP_CENTRIF_EJECUCION "centrif_ejec"       // objName en nextion de si el centrifugado esta activado o no (Si/No)
- NEXTION_COMP_AGUA_EJECUCION "agua_ejec"             // objName en nextion del tipo de agua (Caliente/Fria)

Botones

- NEXTION_COMP_BTN_PARAR "btnParar"                   // objName en nextion del botón de parar
- NEXTION_COMP_BTN_PAUSAR "btnPausar"                 // objName en nextion del botón de pausar

- NEXTION_ID_BTN_PAUSAR 21 // Botón "Pausar"          // ID en nextion del botón de pausar
- NEXTION_ID_BTN_PARAR 22  // Botón "Parar"           // ID en nextion del botón de parar

### Componentes de la pantalla edicion (NEXTION_PAGE_EDIT)

- NEXTION_COMP_PROG_EDICION "progr_sel"          // objName en nextion del programa actual en edición
- NEXTION_COMP_PARAM_EDITAR "param"               // objName en nextion del componente de texto que muestra el parámetro en edición
- NEXTION_COMP_PARAM_VALOR_EDITAR "param_value"   // objName en nextion del componente de texto que muestra el valor del parámetro en edición
- NEXTION_COMP_BTN_TANDA1 "tanda1"                // objName en nextion de componente botón tanda 1
- NEXTION_COMP_BTN_TANDA2 "tanda2"                // objName en nextion de componente botón tanda 2
- NEXTION_COMP_BTN_TANDA3 "tanda3"                // objName en nextion de componente botón tanda 3
- NEXTION_COMP_BTN_TANDA4 "tanda4"                // objName en nextion de componente botón tanda 4

IDs de componentes de control

- NEXTION_ID_BTN_PARAM_MENOS 7      // ID de Botón "-" para disminuir valor del parámetro en edición
- NEXTION_ID_BTN_PARAM_MAS 6        // ID de Botón "+" para aumentar valor del parámetro en edición
- NEXTION_ID_BTN_PARAM_ANTERIOR 8   // ID de Botón anterior para volver al parámetro anterior
- NEXTION_ID_BTN_PARAM_SIGUIENTE 5  // ID de Botón siguiente para pasar al parámetro siguiente
- NEXTION_ID_BTN_GUARDAR 3          // ID de Botón "Guardar"
- NEXTION_ID_BTN_CANCELAR 4         // ID de Botón "Cancelar"
- NEXTION_ID_BTN_TANDA1 26          // ID de Botón "tanda1" (proceso1)
- NEXTION_ID_BTN_TANDA2 27          // ID de Botón "tanda2" (proceso2)
- NEXTION_ID_BTN_TANDA3 28          // ID de Botón "tanda3" (proceso3)
- NEXTION_ID_BTN_TANDA4 29          // ID de Botón "tanda4" (proceso4)

Componentes del panel derecho

- NEXTION_COMP_PARAM_NIVEL_EDIT "val_nivel"     // Componente botón que muestra valor actual del nivel en panel derecho
- NEXTION_COMP_PARAM_TEMP_EDIT "val_temp"       // Componente botón que muestra valor actual de temperatura en panel derecho
- NEXTION_COMP_PARAM_TIEMPO_EDIT "val_tiempo"   // Componente botón que muestra valor actual del tiempo en panel derecho
- NEXTION_COMP_PARAM_CENTRIF_EDIT "val_centrif" // Componente botón que muestra valor actual de centrifugado en panel derecho
- NEXTION_COMP_PARAM_AGUA_EDIT "val_agua"       // Componente botón que muestra valor actual de tipo de agua en panel derecho

IDs de componentes de panel derecho

- NEXTION_ID_PARAM_NIVEL_EDIT 18    // ID de botón que muestra configuración actual del nivel en panel derecho
- NEXTION_ID_PARAM_TEMP_EDIT 19     // ID de botón que muestra configuración actual de temperatura en panel derecho
- NEXTION_ID_PARAM_TIEMPO_EDIT 20   // ID de botón que muestra configuración actual del tiempo en panel derecho
- NEXTION_ID_PARAM_ROTAC_EDIT 21    // ID de botón que muestra configuración actual de rotación en panel derecho
- NEXTION_ID_PARAM_FASE_EDIT 22     // ID de botón que muestra configuración actual de fase en panel derecho
- NEXTION_ID_PARAM_CENTRIF_EDIT 23  // ID de botón que muestra configuración actual de centrifugado en panel derecho
- NEXTION_ID_PARAM_AGUA_EDIT 24     // ID de botón que muestra configuración actual de tipo de agua en panel derecho

### Componentes de la pantalla error (NEXTION_PAGE_ERROR)

- NEXTION_ID_BTN_REINICIAR 3        // ID de Botón "Reiniciar ERROR"
