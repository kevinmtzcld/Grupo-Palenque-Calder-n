# Spec — 05-errores
# Especificación Técnica: Manejo y Recuperación de Errores

## Objetivo
Definir las reglas de detección, clasificación, reporte por consola y mecanismos de recuperación ante errores léxicos, sintácticos y semánticos en el compilador de Razio, garantizando mensajes claros con número de línea y bloqueando la generación de archivos de salida cuando el código fuente presenta fallos.

---

## Alcance
* **Incluye:**
  * **Errores y advertencias léxicas** en `alexico.c`: caracteres fuera del alfabeto, cadenas de texto sin cerrar antes de un salto de línea, desbordamiento de enteros de 32 bits en literales racionales, división por cero en literales (`n/0`) y truncamiento de identificadores mayores a 20 caracteres.
  * **Errores sintácticos** en `sintactico-en-c.c`: reporte mediante `yyerror()` y recuperación en modo pánico mediante el token `error` de Bison hasta `;` o `}`.
  * **Errores semánticos** en `sintactico-en-c.c`: uso de variables no declaradas, redeclaración de variables y llamadas recursivas directas de una función a sí misma.
  * **Control de aborto de compilación:** validación conjunta de las banderas `error_lexico`, `error_sintactico` y `error_semantico` antes de exportar `tabla_simbolos.txt`, `tercetos.txt`, `salida.asm` y `salida.exe`.
* **Excluye:**
  * Errores en tiempo de ejecución (como división por cero dinámica cuando el denominador proviene de una variable evaluada en runtime).

---

## Entradas / Salidas
* **Entradas:**
  * Flujo de caracteres del archivo fuente analizado por `yylex()` (`alexico.c`).
  * Flujo de tokens y reglas gramaticales evaluadas por `yyparse()` (`sintactico-en-c.c`).
  * Estado global de compilación: `num_linea`, `buffer`, `funcion_actual` y `vars_declaradas[]`.
* **Salidas:**
  * Mensajes de diagnóstico formateados en salida estándar (`stdout`) indicando categoría del error, número de línea (`num_linea`) y lexema/contexto involucrado.
  * Activación de banderas globales de estado (`error_lexico = 1`, `error_sintactico = 1`, `error_semantico = 1`).
  * Mensaje final `[ERROR] Se encontraron errores durante el analisis.` sin generación de archivos en disco cuando al menos una bandera de error está activa.

---

## Reglas y decisiones de diseño

### 1. Errores y advertencias en la etapa léxica (`alexico.c`)
| Caso | Acción / Función | Comportamiento y mensaje |
| :--- | :--- | :--- |
| **Carácter inválido** (fuera del alfabeto) | `fe()` | Reporta `[ERROR LEXICO - Linea N]: Caracter no permitido 'X'`, activa `error_lexico = 1`, descarta el carácter en estado `0` sin retracción y continúa el escaneo. |
| **Cadena sin cerrar** (`\n` antes de `"`) | `f7b()` | Reporta `[ERROR LEXICO - Línea N]: Literal cadena sin cerrar antes del salto de linea.`, limpia el buffer, activa `error_lexico = 1` y corta la caminata del estado `13`. |
| **Overflow numérico** (`> 2147483647`, regla D2) | `f3b()` | Detecta desbordamiento de 32 bits con signo en numerador o denominador, reporta `[ERROR LEXICO - Linea N]: Overflow en constante '...' (Excede 32 bits).` y activa `error_lexico = 1`. |
| **División por cero literal** (`a/0`, `a/00`) | `f5()` | Evalúa `atoi(barra + 1) == 0` al cerrar el literal racional; si es cero, reporta `[ERROR LEXICO/SEMANTICO - Linea N]: Division por cero en literal '...'` y activa `error_lexico = 1`. |
| **Identificador largo** (`> 20` caracteres, regla D6) | `f3a()` | Emite `[ADVERTENCIA - Línea N]: ID '...' excede los 20 caracteres y fue truncado.`, trunca a `MAX_ID_LEN = 20` y permite continuar la compilación sin marcar error fatal. |

### 2. Errores y recuperación en la etapa sintáctica (`sintactico-en-c.c`)
* **Reporte centralizado:** Bison invoca `yyerror()`, que imprime `[ERROR SINTACTICO - Linea N]: Cerca de '<buffer>'` y activa `error_sintactico = 1`.
* **Recuperación en modo pánico:** La regla `sentencia` incorpora producciones de sincronización:
  * `error PUNTO_COMA { yyerrok; }`
  * `error LLAVE_DER { yyerrok; }`
  Esto permite descartar tokens hasta el fin de la sentencia o bloque defectuoso y continuar analizando el resto del programa para detectar múltiples errores en una sola pasada.

### 3. Errores en la etapa semántica (`sintactico-en-c.c`)
* **Declaración obligatoria (regla D3):** `verificar_variable()` comprueba que todo identificador usado en asignaciones, expresiones, `mostrar` o `retornar` exista en `vars_declaradas[]`. Caso contrario, reporta `[ERROR SEMANTICO - Linea N]: Variable 'X' no declarada.` y activa `error_semantico = 1`.
* **Prohibición de redeclaración:** `registrar_variable()` impide declarar dos veces el mismo identificador en la sección de declaraciones globales, reportando `[ERROR SEMANTICO - Linea N]: Variable 'X' redeclarada.` y activando `error_semantico = 1`.
* **Prohibición de recursividad directa:** Al invocar `ID()` dentro de `factor` o `llamada_funcion`, se compara `$1` contra `funcion_actual`. Si coinciden, reporta `[ERROR SEMANTICO - Linea N]: La funcion 'X' no puede llamarse a si misma.` y activa `error_semantico = 1`.

### 4. Bloqueo de salida ante errores
* En `main()`, la exportación de `tabla_simbolos.txt`, `tercetos.txt`, `salida.asm` y el ensamblado/enlazado de `salida.exe` están condicionados estrictamente a:
  ```c
  if (error_sintactico == 0 && error_lexico == 0 && error_semantico == 0)
  ```
  Si cualquiera de las tres banderas vale `1`, se omite toda generación de código y archivos de salida.

---

## Criterios de aceptación
* Ante un archivo con caracteres inválidos, cadenas sin cerrar, overflow de 32 bits o literales con denominador `0`, el compilador informa la línea exacta y marca `error_lexico = 1`.
* Ante errores de sintaxis (falta de `;`, paréntesis desbalanceados o estructuras mal formadas), `yyerror()` reporta la línea y la recuperación con `yyerrok` permite seguir analizando las sentencias posteriores.
* Ante variables no declaradas, variables redeclaradas o llamadas recursivas dentro de una función, se emite el mensaje `[ERROR SEMANTICO]` con la línea correspondiente y se marca `error_semantico = 1`.
* Un identificador de más de 20 caracteres emite únicamente una advertencia, se trunca a 20 caracteres y no impide compilar si el resto del programa es válido.
* Ningún archivo de salida (`tabla_simbolos.txt`, `tercetos.txt`, `salida.asm`, `salida.exe`) se genera cuando existe al menos un error léxico, sintáctico o semántico.
