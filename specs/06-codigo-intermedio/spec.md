# Spec — 06-codigo-intermedio

## 1. Objetivo
Esta etapa permite transformar las instrucciones del programa en una serie de operaciones más simples y ordenadas. Los tercetos sirven como paso intermedio entre el análisis del código fuente y la posterior generación del código ensamblador.

## 2. Alcance
Definición de qué tareas se van a realizar en esta etapa del compilador y cuáles quedan fuera.

**Incluye:**

- Definición de la estructura utilizada en C para almacenar los tercetos.
- Generación de tercetos para operaciones aritméticas (`+`, `-`, `*`, `/`) y asignaciones (`=`).
- Representación de las estructuras condicionales (`si` / `sino`) e iterativas (`mientras`).
- Generación de instrucciones para la función de salida `mostrar`.
- Resolución de los destinos de los saltos mediante la técnica de *Backpatching*.
- Exportación de los tercetos generados a un archivo llamado `tercetos.txt`.

**No incluye:**

- Generación del código ensamblador o del código ejecutable final.
- Optimización de los tercetos, como simplificar operaciones o eliminar instrucciones innecesarias.

## 3. Entradas y salidas

**Entradas:**

**Código fuente**: las instrucciones de Razio que Bison reconoce utilizando las reglas gramaticales definidas en sintactico.y.

**Datos de las instrucciones**: los valores necesarios para representar las operaciones, como nombres de variables, números y expresiones matemáticas.

**Tabla de Símbolos**: la información que permite verificar si las variables utilizadas fueron declaradas previamente.

**Salidas:**

**Tabla de tercetos en memoria:** las operaciones se almacenan en tabla_tercetos[] a medida que se procesa el programa.

**Archivo tercetos.txt:** al finalizar el proceso, los tercetos se guardan en un archivo de texto, donde cada uno representa una operación mediante un operador y dos operandos.



Aquí tenés la especificación completa de generación de código intermedio adaptada **exactamente** a tu lenguaje **Razio**, a tu gramática en Bison (`sintactico-en-c.c`), a tu tabla `tabla_tercetos` (`[Tn] (op, op1, op2)`) y a tus reglas reales (propagación directa mediante `%union` con `$$` y `$1`, evaluación lógica completa sin cortocircuito, funciones sin parámetros y salto inicial al `principal`):

---

### 4. Estructura de Tercetos por Regla Gramatical (Lenguaje Razio)

**Propagación de Valores y Uso de Marcadores:** Como Bison (`sintactico-en-c.c`) ejecuta las acciones semánticas al reducir cada producción, los lexemas de identificadores, literales y referencias a tercetos generados (`[Tn]`) se transportan hacia arriba mediante el campo `str_val[50]` definido en `%union` (`strcpy($$, $1)` o `sprintf($$, "[T%d]", pos)`). Para las estructuras de control que requieren emitir saltos tempranos o guardar posiciones antes de reducir el bloque (`si`, `sino`, `mientras`), se utilizan reglas intermedias (`marca_si`, `inicio_mientras`, `condicion_mientras`) y acciones embebidas que apilan y desapilan índices en `pila_saltos` (*backpatching*).

#### 4.1 Operandos simples y Reglas intermedias

* **Operandos (`factor: LIT_RACIONAL | ID`):** No generan tercetos.

* **Acción:**
* Si es un literal racional (`LIT_RACIONAL`), se copia su representación textual (`$1`, ej. `"3/4"`) hacia el símbolo no terminal `factor` mediante `strcpy($$, $1)`.

* Si es un identificador (`ID`), se valida mediante `verificar_variable($1)` que haya sido declarado previamente y se propaga su nombre hacia arriba (`strcpy($$, $1)`).

* **Reglas intermedias (`termino: factor`, `expresion: termino`, `condicion: cond_or`, `cond_or: cond_and`, `cond_and: cond_factor`, `cond_factor: comparacion`):** No generan tercetos ni alteran la pila de saltos.

* **Acción:** Únicamente propagan hacia arriba el lexema o la referencia al terceto ya creado (`strcpy($$, $1)`).

* **Agrupación con paréntesis (`factor: PAR_IZQ expresion PAR_DER`, `cond_factor: PAR_IZQ condicion PAR_DER`):** No generan tercetos.

* **Acción:** Descartan los paréntesis propagando directamente la referencia interna (`strcpy($$, $2)`).

#### 4.2 Declaración e Inicialización de Variables

* **Declaración simple (`variable: ID`):** No genera tercetos.

* **Acción:** Registra el identificador en la tabla de variables declaradas (`registrar_variable($1)`).

* **Declaración con inicialización positiva (`variable: ID OP_ASIG LIT_RACIONAL`):
* **Acción:** Registra la variable y genera el terceto de asignación inicial `(=, ID, LIT_RACIONAL)` (ej. `[T0] (=, a, 1/2)`).

* **Declaración con inicialización negativa (`variable: ID OP_ASIG OP_RESTA LIT_RACIONAL`):
* **Acción:** Registra la variable, concatena el signo negativo con el literal (`-$4`) y genera el terceto `(=, ID, -LIT_RACIONAL)` (ej. `[T1] (=, b, -2/1)`).

#### 4.3 Expresiones Aritméticas y Relacionales

* **Operaciones aritméticas (`expresion: expresion OP_SUMA termino | expresion OP_RESTA termino` y `termino: termino OP_MULT factor | termino OP_DIV factor`):**
* **Acción:** Toma los operandos propagados desde `$1` y `$3`, genera el terceto correspondiente con los operadores `+`, `-`, `*` o `/` mediante `crear_terceto(op, $1, $3)` (ej. `[T2] (+, a, 3/4)`) y guarda la referencia `"[T2]"` en `$$` (`sprintf($$, "[T%d]", pos)`).

* **Operador de comparación (`operador_comparacion: COMP_MAYOR | COMP_MENOR | COMP_MAYOR_IGUAL | COMP_MENOR_IGUAL | COMP_IGUAL | COMP_DISTINTO`):** No genera tercetos.

* **Acción:** Convierte el token relacional en su cadena simbólica (`">"`, `"<"`, `">="`, `"<="`, `"=="`, `"!="`) y la guarda en `$$`.

* **Comparaciones (`comparacion: expresion operador_comparacion expresion`):**
* **Acción:** Genera el terceto relacional `crear_terceto($2, $1, $3)` (ej. `[T3] (>, a, 5/1)`) y propaga su referencia `"[T3]"` en `$$`.

#### 4.4 Asignaciones

* **Regla (`asignacion: ID OP_ASIG expresion PUNTO_COMA`):**
* **Acción:** Verifica que la variable destino exista (`verificar_variable($1)`) y genera el terceto de asignación utilizando la referencia propagada en `$3`: `crear_terceto("=", $1, $3)` (ej. `[T4] (=, x, [T2])`).

#### 4.5 Operadores Lógicos (`and` / `or`)

El compilador implementa **evaluación lógica completa** (sin cortocircuito), resolviendo cada subcondición a un valor booleano temporal (`[Tn]`) antes de emitir el salto condicional.

* **Operador lógico AND (`cond_and: cond_and OP_AND cond_factor`):**
* **Acción:** Toma las referencias booleanas de ambos operandos (`$1` y `$3`), genera el terceto lógico `crear_terceto("AND", $1, $3)` (ej. `[T5] (AND, [T3], [T4])`) y propaga su referencia `"[T5]"` en `$$`.

* **Operador lógico OR (`cond_or: cond_or OP_OR cond_and`):**
* **Acción:** Toma las referencias booleanas de ambos operandos (`$1` y `$3`), genera el terceto lógico `crear_terceto("OR", $1, $3)` (ej. `[T6] (OR, [T5], [T2])`) y propaga su referencia `"[T6]"` en `$$`.

#### 4.6 Estructura Condicional `si` / `sino`

* **Al reducir la condición (`marca_si: PR_SI PAR_IZQ condicion PAR_DER`):**
* **Acción:** Genera el terceto de Bifurcación por Falso con destino pendiente: `crear_terceto("BF", $3, "?")` y apila su índice (`pos_bf`) en `pila_saltos` mediante `apilar(pos_bf)`.

* **Caso `si` simple (`si: marca_si bloque`):**
* **Acción:** Al finalizar el bloque verdadero, desapila `pos_bf` de `pila_saltos` y completa su segundo operando con la dirección del próximo terceto a generar (`modificar_terceto(pos_bf, "[T_fin]")`).

* **Caso `si` con `sino` (`si: marca_si bloque PR_SINO {A1} bloque {A2}`):**
* **En `{A1}` (al finalizar el bloque `si`, antes de entrar al `sino`):**
1. Desapila `pos_bf` de `pila_saltos`.
2. Genera un terceto de Bifurcación Incondicional con destino pendiente para esquivar el bloque `sino`: `crear_terceto("BI", "?", "-")`, y apila su índice (`pos_bi`).
3. Completa el destino de `pos_bf` apuntando al inicio del bloque `sino` (`modificar_terceto(pos_bf, "[T_sino]")`).

* **En `{A2}` (al finalizar el bloque `sino`):**
1. Desapila `pos_bi` de `pila_saltos` y completa su primer operando (`modificar_terceto_op1(pos_bi, "[T_fin]")`) para continuar la ejecución fuera de la estructura.

#### 4.7 Estructura Iterativa `mientras`

* **Al iniciar el bucle (`inicio_mientras: PR_MIENTRAS`):**
* **Acción:** Guarda en `pila_saltos` el índice del próximo terceto a generar (`apilar(indice_terceto)`), marcando el punto exacto donde comienza a evaluarse la condición.

* **Al reducir la condición (`condicion_mientras: inicio_mientras PAR_IZQ condicion PAR_DER`):**
* **Acción:** Genera el terceto de salida por falso con destino pendiente `crear_terceto("BF", $3, "?")` y apila su índice (`pos_bf`) en `pila_saltos`.

* **Al finalizar el bloque del bucle (`mientras: condicion_mientras bloque`):**
* **Acción:** Desapila dos índices de `pila_saltos`:
1. El primer valor desapilado (`pos_bf`) corresponde al salto de salida.
2. El segundo valor desapilado (`pos_inicio`) corresponde al comienzo de la condición. Con él se genera el salto incondicional de retorno: `crear_terceto("BI", "[T_inicio]", "-")`.
3. Finalmente, se completa el destino pendiente de `pos_bf` con el índice del terceto siguiente al `BI` (`modificar_terceto(pos_bf, "[T_fin]")`).

#### 4.8 Sentencias de Salida y Retorno (`mostrar` / `retornar`)

* **Sentencia `mostrar` (`argumentos_mostrar` / `lista_ids_mostrar`):**
* **Variante Cadena (`argumentos_mostrar: LIT_CADENA`):** Genera `crear_terceto("MOSTRAR_STR", $1, "-")`.

* **Variante Variable (`argumentos_mostrar: ID` / `lista_ids_mostrar: ID | lista_ids_mostrar COMA ID`):** Verifica que la variable esté declarada y genera `crear_terceto("MOSTRAR_ID", ID, "-")`.

* **Variante Mixta (`argumentos_mostrar: LIT_CADENA {A1} COMA lista_ids_mostrar`):** En `{A1}` emite primero `crear_terceto("MOSTRAR_STR", $1, "-")` y luego cada identificador de `lista_ids_mostrar` emite su propio terceto `MOSTRAR_ID` en orden de aparición.

* **Sentencia `retornar` (`retornar: PR_RETORNAR PAR_IZQ valor_retorno PAR_DER PUNTO_COMA`):**
* **Acción:** La regla auxiliar `valor_retorno` (`ID | LIT_RACIONAL`) propaga el valor en `$$`, y `retornar` genera `crear_terceto("RETORNAR", $3, "-")`.

#### 4.9 Estructura del Programa, Declaración y Llamado de Funciones

En **Razio**, las funciones no reciben parámetros y el bloque `principal()` constituye el punto de entrada obligatorio del programa.

* **Inicio del programa y bloque `principal` (`programa: declaraciones {A1} funciones PR_PRINCIPAL PAR_IZQ PAR_DER {A2} bloque`):**
* **En `{A1}` (tras las declaraciones globales):** Genera `crear_terceto("IR_PRINCIPAL", "-", "-")` para saltar por encima de las definiciones de funciones auxiliares.

* **En `{A2}` (al abrir `principal()`):** Fija `funcion_actual = "principal"` y genera la etiqueta de entrada `crear_terceto("PRINCIPAL", "-", "-")`.

* **Definición de función (`funcion: PR_DEF ID PAR_IZQ PAR_DER {A1} bloque {A2}`):**
* **En `{A1}`:** Fija `funcion_actual = $2` y genera la marca de inicio `crear_terceto("FUNC_INICIO", $2, "-")`.

* **En `{A2}`:** Genera la marca de cierre `crear_terceto("FUNC_FIN", $2, "-")` y limpia `funcion_actual`.

* **Llamado como sentencia (`llamada_funcion: ID PAR_IZQ PAR_DER PUNTO_COMA`):**
* **Acción:** Verifica que la función no se llame recursivamente (`$1 != funcion_actual`) y genera `crear_terceto("CALL", $1, "-")`.

* **Llamado dentro de una expresión (`factor: ID PAR_IZQ PAR_DER`):**
* **Acción:** Verifica no recursividad, genera `pos = crear_terceto("CALL", $1, "-")` y propaga su referencia `"[T%d]"` en `$$` para que su valor de retorno sea operado en la expresión.




















## 5. Criterios de aceptación

Para considerar que esta etapa funciona correctamente, se deben cumplir las siguientes condiciones:

1. **Orden correcto de las operaciones:** los tercetos deben generarse respetando la prioridad de los operadores. Por ejemplo, en `x = a + b * c`, primero debe generarse la multiplicación y después la suma.

2. **Formato del archivo:** cuando el análisis del programa finaliza correctamente, se debe generar el archivo `tercetos.txt`, con cada terceto en una línea y siguiendo el formato `[T_] (op, op1, op2)`.

3. **Saltos correctamente resueltos:** las instrucciones `BF` y `BI` deben tener indicado un destino válido dentro de la tabla de tercetos.

4. **Control de errores:** si el programa contiene errores léxicos o sintácticos (`error_lexico != 0` o `error_sintactico != 0`), no se debe generar ni exportar el archivo de tercetos.

