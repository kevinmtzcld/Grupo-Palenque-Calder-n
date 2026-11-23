# Spec — 02-tabla-simbolos
# Especificación Técnica: Tabla de Símbolos

## 1. Descripción General
La **Tabla de Símbolos (TS)** es la estructura de datos en memoria encargada de almacenar y gestionar la información relevante sobre los identificadores (`ID`) y constantes de texto (`LIT_CADENA`) o numéricas (`LIT_RACIONAL`) reconocidas por el analizador léxico durante la lectura del código fuente en lenguaje Razio.

---

## 2. Estructura de Datos en C (`RegTablaSimbolos`)

En el archivo `alexico.c`, la Tabla de Símbolos está implementada mediante un arreglo estático de estructuras (`struct`) con una capacidad máxima de 500 registros:

```c
#define MAX_BUFFER 256

typedef struct {
    char nombre[MAX_BUFFER];  /* Lexema original o valor textual */
    char tipo[20];            /* Categoría: "ID", "LIT_RACIONAL" o "LIT_CADENA" */
    char valor[MAX_BUFFER];   /* Valor asignado o valor racional normalizado */
    int longitud;             /* Largo en caracteres del lexema */
} RegTablaSimbolos;

/* Arreglo global de la Tabla de Símbolos */
RegTablaSimbolos tabla_simbolos[500];
int cant_simbolos = 0;
```

---

## Objetivo
Centralizar el registro único de identificadores, literales racionales y literales de cadena detectados en el programa fuente, evitando duplicados, normalizando los valores numéricos y dejando disponible tanto una estructura consultable en tiempo de compilación como una exportación legible para verificación y generación de código.

---

## Alcance
* **Incluye:**
  * Registro en memoria de símbolos de categoría `ID`, `LIT_RACIONAL` y `LIT_CADENA` mediante `insertar_ts()`.
  * Prevención de entradas repetidas por lexema (`nombre`).
  * Normalización de literales enteros a formato racional `n/1` en el campo `valor`.
  * Cálculo y almacenamiento de la longitud en caracteres del lexema.
  * Visualización tabular por consola y exportación a archivo plano (`tabla_simbolos.txt`) mediante `exportarTS()`.
  * Tabla auxiliar de declaraciones (`vars_declaradas` en `sintactico-en-c.c`) para controlar declaraciones obligatorias y redeclaraciones a nivel semántico.
* **Excluye:**
  * Palabras reservadas (`def`, `principal`, `racional`, `si`, `sino`, `mientras`, `mostrar`, `retornar`), operadores lógicos (`and`, `or`), operadores aritméticos/relacionales y signos de puntuación (se resuelven como tokens sin ingresar a la TS).
  * Comentarios multilínea `/* ... */` y caracteres blancos/separadores.

---

## Entradas / Salidas
* **Entradas:**
  * Lexemas validados por las acciones semánticas del autómata léxico (`f4` para identificadores, `f5` para literales racionales y `f7c` para literales de cadena).
  * Parámetros recibidos por `insertar_ts(nombre, tipo, valor, longitud)`.
* **Salidas:**
  * Estructura global en memoria `tabla_simbolos[]` (hasta 500 entradas) con su contador `cant_simbolos`.
  * Reporte formateado en salida estándar (`stdout`) con columnas `NOMBRE`, `TIPO`, `VALOR` y `LONGITUD`.
  * Archivo CSV plano `tabla_simbolos.txt` con cabecera `NOMBRE,TIPO,VALOR,LONGITUD` seguido de una línea por cada símbolo registrado.

---

## Reglas y decisiones de diseño
1. **Unicidad por lexema (`nombre`):** Antes de insertar un registro, `insertar_ts()` recorre secuencialmente `tabla_simbolos[0..cant_simbolos-1]` comparando con `strcmp`. Si el lexema ya existe, descarta la inserción para evitar duplicados.
2. **Capacidad fija:** El arreglo soporta hasta `500` símbolos (`MAX_BUFFER = 256` caracteres para `nombre` y `valor`, y `20` caracteres para `tipo`). Si se alcanza el límite de 500 entradas, no se agregan nuevos registros.
3. **Convención de atributos por categoría:**
   * **`ID`:** `nombre` guarda el identificador (truncado a `MAX_ID_LEN = 20` caracteres según regla D6), `tipo = "ID"`, `valor = "-"` y `longitud = strlen(buffer)`.
   * **`LIT_RACIONAL`:** `nombre` conserva el lexema original tal como aparece en el código fuente (ej. `"5"` o `"3/4"`), `tipo = "LIT_RACIONAL"`, `valor` guarda el racional normalizado (`"5/1"` si no tenía barra `/` o `"3/4"` si ya era fracción) y `longitud = strlen(buffer)`.
   * **`LIT_CADENA`:** `nombre` y `valor` almacenan el contenido textual sin las comillas delimitadoras, `tipo = "LIT_CADENA"` y `longitud = strlen(buffer)`.
4. **Momento de exportación:** La función `exportarTS()` se invoca desde `main()` únicamente cuando el análisis léxico, sintáctico y semántico finaliza sin errores (`error_lexico == 0 && error_sintactico == 0 && error_semantico == 0`).
5. **Separación entre TS léxica y control de declaraciones:** Mientras `tabla_simbolos` registra todo identificador léxicamente válido para inspección y generación de datos en assembler, `sintactico-en-c.c` mantiene el arreglo `vars_declaradas[500][50]` con `registrar_variable()` y `verificar_variable()` para detectar variables no declaradas o redeclaradas (regla D3).

---

## Criterios de aceptación
* Cada identificador, literal racional y literal de cadena válido del código fuente aparece exactamente una vez en `tabla_simbolos.txt`, sin duplicados ante usos repetidos.
* Las palabras reservadas, operadores y delimitadores no aparecen registrados en la Tabla de Símbolos.
* Los identificadores que superan los 20 caracteres se truncan a 20 antes de insertarse y reflejan `longitud = 20`.
* Los literales numéricos escritos sin denominador (ej. `2`) registran `nombre = "2"` y `valor = "2/1"`, mientras que los escritos con barra (ej. `3/4`) conservan `valor = "3/4"`.
* Los literales de cadena se almacenan sin comillas y con su longitud real de caracteres internos.
* Si el programa fuente contiene errores léxicos, sintácticos o semánticos, el compilador aborta antes de sobreescribir o exportar `tabla_simbolos.txt`.
