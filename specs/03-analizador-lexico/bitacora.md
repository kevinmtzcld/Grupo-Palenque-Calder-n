# Bitácora de Desarrollo del Analizador Léxico (Razio)

En este documento se registra la evolución del desarrollo del Analizador Léxico para el lenguaje **Razio**, documentando las iteraciones de trabajo con el modelo de IA (Gemini), los problemas identificados, las decisiones de diseño tomadas y su alineación con las especificaciones técnicas de la cátedra.

---

## Iteración 0 — Generación inicial de la estructura y reglas del analizador léxico

- **Fecha:** 20/09/2026 · Gemini
- **Qué pedí:** Proporcioné la documentación base de la cátedra sobre el lenguaje Razio (`paralexico.txt`, la tabla de tokens, las clases del alfabeto y el significado conceptual de las acciones semánticas $f1$ a $f8$)[cite: 9]. Pedí generar el primer borrador en C del analizador léxico (`lexico.c`), el archivo de cabecera (`lexico.h`), las definiciones de tokens (`tokens.h`) y un `main.c` con salida tabular básica.
- **Qué devolvió:** Un código estructurado en múltiples archivos C con las funciones semánticas implementadas, un buffer de acumulación, lógica de truncado de identificadores, reconocimiento de constantes racionales, palabras reservadas y manejo de literales de cadena. Para la selección de estados ejecutaba un bloque `switch(estado_actual)` dentro de la función de análisis.
- **Qué cambié y por qué:** Se probó en un entorno de compilación C local / OnlineGDB con un archivo de prueba `ejemplo.raz`. Aunque reconocía los tokens y los mostraba en pantalla, en la revisión posterior el profesor señaló dos correcciones estructurales fundamentales:
  1. Estaba prohibido usar condicionales o `switch` para seleccionar la acción semántica; se debía usar obligatoriamente la matriz de punteros a función[cite: 4, 6].
  2. Los archivos de cabecera `lexico.h` y `tokens.h` representaban un sobre-diseño prematuro para la entrega del léxico puro.
- **Impacto en la spec:** Se tomó la decisión de unificar el código en un solo módulo ejecutable y migrar toda la lógica del AFD a la arquitectura basada en la matriz de funciones `proceso[estado][columna]`[cite: 4, 6].

---

## Iteración 1 — Corrección de arquitectura y unificación de archivos

- **Fecha:** 27/09/2026 · Gemini
- **Qué pedí:** Corregir las observaciones del profesor sobre el código inicial (falta de uso de la matriz de punteros a función y la objeción sobre el uso prematuro de `lexico.h` y `tokens.h`). Además, reorganizar la estructura de carpetas del repositorio de GitHub (`src/`, `tests/`, `specs/`, `out/`).
- **Qué devolvió:** Una versión unificada en un solo archivo C (`main.c`) utilizando `typedef void (*AccionSemantica)(void)` y la matriz `proceso[28][20]` invocada dinámicamente mediante `(*proceso[estado][columna])()`, eliminando los archivos de cabecera separados[cite: 4, 6].
- **Qué cambié y por qué:** Se aceptó la reestructuración para cumplir estrictamente con el modelo teórico y el pseudocódigo de la cátedra[cite: 4, 6], donde la matriz ejecuta directamente las funciones semánticas ($f1–f8, fn, fe$) en lugar de simularlas con `switch-case` o condicionales[cite: 6].
- **Impacto en la spec:** Se organizó el repositorio colocando `spec.md` en `specs/01-diseno/` para volcar la documentación técnica del lenguaje Razio[cite: 3].

---

## Iteración 2 — Limpieza del buffer en delimitadores y operadores simples

- **Fecha:** 27/09/2026 · Gemini
- **Qué pedí:** Analizar el resultado del reporte de tokens tras probar un programa de prueba integral (`ejemplo.raz`), donde los delimitadores (`(`, `)`, `{`, `}`, `;`, `,`) y asignaciones (`=`) mostraban un lexema desfasado (el texto del token anterior).
- **Qué devolvió:** Identificó que la función nula `fn()` no estaba actualizando ni limpiando el `buffer` al transicionar sobre caracteres unitarios de un solo paso.
- **Qué cambié y por qué:** Modificamos `fn()` para que guarde explícitamente el carácter actual en el `buffer` cuando este se encuentre vacío y no sea un espacio en blanco, y agregamos el reinicio de `buf_idx = 0` al inicio de cada llamada en `yylex()`. Esto corrigió la salida permitiendo que cada operador o delimitador reporte su propio lexema.
- **Impacto en la spec:** Ninguno.

---

## Iteración 3 — Ajuste a las restricciones de la cátedra (32 bits, truncamiento y Tabla de Símbolos)

- **Fecha:** 27/09/2026 · Gemini
- **Qué pedí:** Revisar si el código contenía generalizaciones u omisiones respecto a las especificaciones completas de `paralexico.txt`[cite: 9], `TP_00_Consignas_Generales.pdf`[cite: 7, 8] y `TP_Grupo_C.pdf`[cite: 8, 9].
- **Qué devolvió:** Una actualización que ajustó el límite máximo de identificadores a 20 caracteres con advertencia de truncamiento (Decisión D6)[cite: 9], agregó la validación de overflow en tiempo real para racionales de 32 bits ($MAX\_INT\_32 = 2.147.483.647$, Decisión D2)[cite: 9], la detección de denominador cero (`/0`, Decisión D9)[cite: 9] y la exportación física de la Tabla de Símbolos a `tabla_simbolos.txt`[cite: 8].
- **Qué cambié y por qué:** Se adoptaron todas las correcciones para garantizar que el léxico cumpla tanto con las pautas generales como con el eje temático específico del Grupo C (manejo de tipo racional exacto de 32 bits y detección de errores)[cite: 8, 9].
- **Impacto en la spec:** Se alineó el código C con las decisiones $D1$ a $D10$ definidas en `paralexico.txt`[cite: 9].

---

## Iteración 4 — Salida tabular de tokens y alineación exacta con el pseudocódigo del main

- **Fecha:** 27/09/2026 · Gemini
- **Qué pedí:** Formatear la salida de los tokens en una tabla estructurada por columnas y verificar la coincidencia absoluta del bucle de lectura con la diapositiva en pseudocódigo del profesor[cite: 4, 10].
- **Qué devolvió:** Implementó el formateador en consola con encabezados (`LÍNEA`, `TOKEN`, `CÓDIGO`, `LEXEMA`) y reestructuró la interacción de lectura entre `main()` (mediante el bucle `while (c_actual_main != EOF)`) y `yylex()`.
- **Qué cambié y por qué:** Se aceptó el cambio para que la salida sea totalmente legible durante la corrección[cite: 8] y la arquitectura de llamadas coincida en un 100% con el diagrama presentado por la cátedra[cite: 4, 10].
- **Impacto en la spec:** Ninguno.
