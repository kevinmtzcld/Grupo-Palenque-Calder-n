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


---

### 4. Estructura de Tercetos por Regla Gramatical (Lenguaje Razio)

* **Propagación de Valores y Uso de Marcadores:**
  * **Expresiones aritméticas:** Cada regla propaga el operando resultante (el nombre de la variable, el valor literal o la referencia `[Tx]` del terceto recién creado) para que las operaciones siguientes puedan utilizarlo.
  * **Condiciones lógicas:** Las comparaciones y conectores (`and`, `or`) transmiten dos listas de saltos pendientes (`truelist` y `falselist`), permitiendo resolver la evaluación por cortocircuito.
  * **Marcadores y *backpatching*:** Como Bison traduce en una sola pasada de izquierda a derecha y aún no conoce el destino de los saltos cuando los emite, las estructuras de control (`si`, `sino`, `mientras`) utilizan reglas marcador (`M`, `marca_si`, `inicio_mientras`, `condicion_mientras`) para guardar posiciones clave. De esta forma, los saltos (`BF`, `BV`, `BI`) se crean con destino pendiente y se rellenan automáticamente una vez que se conoce a qué terceto deben apuntar.

**Convención de Variables Semánticas de Bison (`$$`, `$1`, `$2`, ...)**

Para interpretar las acciones semánticas de cada producción, se utilizan los siguientes marcadores:
```text
NoTerminal  :  Simbolo_1   Simbolo_2   {accion_intermedia}   Simbolo_3
    $$             $1          $2               $3               $4

- **`$$`:** Representa el valor semántico del símbolo no terminal ubicado a la izquierda de la producción. Se utiliza para guardar el resultado y propagarlo hacia las reglas gramaticales superiores.
- **`$1`, `$2`, `$3`, ...:** Representan los valores semánticos de los símbolos que están a la derecha de la producción. El número indica la posición que ocupa cada símbolo, contando de izquierda a derecha.

#### 4.1 Operandos simples y Reglas intermedias

- **Operandos (`factor: LIT_RACIONAL | ID`):** No generan tercetos.
  - **Acción:** Si es un literal racional (`LIT_RACIONAL`), se copia su representación textual (`$1`, ej. `"3/4"`) hacia el símbolo no terminal `factor` mediante `strcpy($$, $1)`.
  - **Acción:** Si es un identificador (`ID`), se valida mediante `verificar_variable($1)` que haya sido declarado antes y se propaga su nombre hacia arriba (`strcpy($$, $1)`).

- **Reglas intermedias:**
  - **Aritméticas (`termino: factor`, `expresion: termino`):** No generan tercetos. Transmiten hacia arriba el lexema o la referencia al terceto creado mediante `strcpy($$, $1)`.
  - **Lógicas (`condicion: cond_or`, `cond_or: cond_and`, `cond_and: cond_factor`, `cond_factor: comparacion`):** No generan tercetos. Propagan hacia arriba la estructura `CondAttr`, que contiene las listas `truelist` y `falselist`, mediante `$$ = $1`.

- **Agrupación con paréntesis:**
  - **Aritmética (`factor: PAR_IZQ expresion PAR_DER`):** Descarta los paréntesis y propaga la referencia interna mediante `strcpy($$, $2)`.
  - **Lógica (`cond_factor: PAR_IZQ condicion PAR_DER`):** Descarta los paréntesis y propaga las listas `truelist` y `falselist` de la condición interna mediante `$$ = $2`.

#### 4.2 Declaración e Inicialización de Variables

- **Declaración simple (`variable: ID`):** No genera tercetos.
  - **Acción:** Registra el identificador en la tabla de variables declaradas (`registrar_variable($1)`).

- **Declaración con inicialización positiva (`variable: ID OP_ASIG LIT_RACIONAL`):**
  - **Acción:** Registra la variable y genera el terceto de asignación inicial `(=, ID, LIT_RACIONAL)` (ej. `[T0] (=, a, 1/2)`).

#### 4.3 Expresiones Aritméticas y Comparaciones

- **Operaciones aritméticas (`expresion: expresion OP_SUMA termino | expresion OP_RESTA termino` y `termino: termino OP_MULT factor | termino OP_DIV factor`):**
  - **Acción:** Toma los dos operandos de la operación: `$1` contiene el operando izquierdo y `$3` el operando derecho. Según el operador reconocido (`+`, `-`, `*` o `/`), genera el terceto correspondiente mediante `crear_terceto(op, $1, $3)` (ej. `[T2] (+, a, 3/4)`). Luego, guarda la referencia al terceto generado (`[T2]`) en `$$`, para que pueda utilizarse en operaciones superiores.

- **Operador de comparación (`operador_comparacion: COMP_MAYOR | COMP_MENOR | COMP_MAYOR_IGUAL | COMP_MENOR_IGUAL | COMP_IGUAL | COMP_DISTINTO`):**
  - **Acción:** No genera ningún terceto. Según el token reconocido, obtiene el símbolo correspondiente (`>`, `<`, `>=`, `<=`, `==`, `!=`) y lo guarda en `$$`. De esta manera, la siguiente regla puede utilizar ese símbolo para generar el terceto de comparación.

- **Comparaciones (`comparacion: expresion operador_comparacion expresion`):**
  - **Acción:** Genera el terceto de comparación `pos_comp = crear_terceto($2, $1, $3)` (ej. `[T0] (>, x, 5/1)`). Luego, genera un salto condicional por falso `pos_bf = crear_terceto("BF", "[T0]", "?")`, cuyo destino queda pendiente. Finalmente, inicializa `$$.truelist` vacía y guarda `pos_bf` en `$$.falselist` para completar el destino del salto cuando se conozca.

#### 4.4 Asignaciones

- **Regla (`asignacion: ID OP_ASIG expresion PUNTO_COMA`):**
  - **Acción:** Verifica que la variable destino esté declarada con `verificar_variable($1)` y genera el terceto de asignación `crear_terceto("=", $1, $3)`, donde `$1` es la variable destino y `$3` es el valor de la expresión (ej. `[T4] (=, x, [T2])`).

#### 4.5 Operadores Lógicos (`and` / `or`) con Cortocircuito

El compilador no genera tercetos intermedios (`AND`/`OR`), sino que resuelve la evaluación lógica mediante cortocircuito y *backpatching* sobre las listas `truelist` y `falselist`:

- **Marcador de posición (`M: /* vacio */`):**
  - **Acción:** Guarda en `$$` el índice del próximo terceto a generarse (`$$ = indice_terceto`). Marca el punto de entrada de la segunda subcondición.

* **Operador lógico AND (`cond_and: cond_and OP_AND M cond_factor`):**
  * **Acción semántica:** Resuelve la evaluación por cortocircuito encadenando ambas condiciones de izquierda a derecha mediante tres operaciones:
    * **Continuación por verdadero:** Si la primera condición (`$1`) se cumple, se completan sus saltos verdaderos apuntando al marcador `M` (`backpatch($1.truelist, $3)`), para que el flujo avance directamente a evaluar la segunda condición (`$4`).
    * **Salida por verdadero del conjunto:** Como el `AND` completo solo es verdadero si también se cumple la segunda condición, la expresión hereda únicamente los saltos verdaderos de esta última (`$$.truelist = $4.truelist`), dejándolos pendientes hasta conocer el inicio del bloque.
    * **Salida por falso (cortocircuito):** Si cualquiera de las dos condiciones falla, toda la expresión es falsa; por eso se unifican ambas listas de saltos por falso (`$$.falselist = combinar_listas($1.falselist, $4.falselist)`). De este modo, si `$1` falla, salta sin evaluar `$4`, y ambas salidas terminarán apuntando al mismo destino falso final.
      

* **Operador lógico OR (`cond_or: cond_or OP_OR {A1} M cond_and`):**
  * **Acción semántica:** Resuelve la evaluación por cortocircuito asegurando que alcance con que se cumpla una de las dos condiciones, mediante dos momentos de ejecución:
    * **Cortocircuito por verdadero (`{A1}`):** Antes de evaluar la segunda condición (`$5`), convierte el salto por falso (`BF`) que se había generado originalmente para la primera condición (`$1`) en un salto por verdadero (`BV`) y lo pasa de `$1.falselist` a `$1.truelist`. De esta forma, si la primera condición se cumple, salta directamente al bloque verdadero sin evaluar la segunda.
    * **Continuación por falso:** Si la primera condición falla, se rellenan sus saltos falsos restantes con la posición del marcador `M` (`backpatch($1.falselist, $4)`), haciendo que el flujo avance a evaluar la segunda condición (`$5`).
    * **Resultado final del conjunto:** Como el `OR` es verdadero si se cumple cualquiera de las dos condiciones, junta ambas listas verdaderas en una sola (`$$.truelist = combinar_listas($1.truelist, $5.truelist)`) para rellenarlas juntas cuando se conozca el inicio del bloque. En cambio, toda la expresión falla únicamente si también falla la segunda condición, por lo que guarda sus saltos falsos pendientes (`$$.falselist = $5.falselist`) para rellenarlos más adelante con el destino falso final.
   
  
#### 4.6 Estructura Condicional `si` / `sino`

* **Al terminar de leer la condición (`marca_si: PR_SI PAR_IZQ condicion PAR_DER`):**
  * **Acción:** Justo antes de entrar al bloque `{ ... }`, `condicion` (`$3`) trae dos listas de índices de saltos pendientes (`?`): `truelist` y `falselist`. En este punto se conoce el inicio del bloque pero aún no su final:
    * **Resuelve los saltos verdaderos:** completa `$3.truelist` con el terceto actual (`backpatch($3.truelist, indice_terceto)`), entrando directo al inicio del bloque.
    * **Conserva los saltos falsos:** guarda `$3` en `pila_condiciones` (`apilar_cond($3)`) para conservar su `falselist` mientras se compila el interior (y permitir `si` anidados).

* **Al cerrar un `si` simple (`si: marca_si bloque`):**
  * **Acción:** Al terminar de compilar el bloque `{ ... }`, el terceto actual es la primera instrucción posterior a la estructura:
    * **Completa la salida por falso:** desapila `c` desde `pila_condiciones` y rellena `c.falselist` con el terceto actual (`backpatch(c.falselist, indice_terceto)`), salteando el bloque si la condición falló.

* **Caso `si` con `sino` (`si: marca_si bloque PR_SINO {A1} bloque {A2}`):**
  * **En `{A1}` (al finalizar el bloque `si`, antes de entrar al `sino`):**
    1. Desapila `c` de `pila_condiciones` para recuperar su `falselist`.
    2. Genera un salto incondicional pendiente (`crear_terceto("BI", "?", "-")`) y apila su índice (`pos_bi`) en `pila_saltos` para saltear el `sino` en caso de que se haya ejecutado el bloque verdadero del `si`
    3. Completa `c.falselist` apuntando al terceto actual (`backpatch(c.falselist, indice_terceto)`), marcando el inicio del bloque `sino`.

  * **En `{A2}` (al finalizar el bloque `sino`):**
    1. Desapila `pos_bi` de `pila_saltos` y completa su destino con el terceto de salida (`modificar_terceto_op1(pos_bi, "[T_fin]")`), para saltear u omitir el bloque `sino`



#### 4.7 Estructura Iterativa `mientras`

* **Al iniciar el bucle (`inicio_mientras: PR_MIENTRAS`):**
  * **Acción:** Antes de compilar la condición, guarda en `pila_saltos` el número del terceto actual (`apilar(indice_terceto)`), dejando marcada la posición exacta donde deberá volver la ejecución al final de cada iteración para reevaluar la condición.

* **Al terminar de leer la condición (`condicion_mientras: inicio_mientras PAR_IZQ condicion PAR_DER`):**
  * **Acción:** Justo antes de entrar al bloque `{ ... }`, ya se conoce dónde empieza el cuerpo del bucle pero aún no dónde termina:
    * **Resuelve los saltos verdaderos:** Completa los saltos verdaderos pendientes de `condición` `$3.truelist` con el terceto actual -> (`backpatch($3.truelist, indice_terceto)`), ingresando directo al cuerpo del `mientras`.
    * **Conserva los saltos falsos:** guarda `$3` en `pila_condiciones` (`apilar_cond($3)`) para conservar su `falselist` mientras se compila el interior (y permitir bucles anidados).

* **Al cerrar el bloque del bucle (`mientras: condicion_mientras bloque`):**
  * **Acción:** Una vez compilado el cuerpo `{ ... }`, se cierra el ciclo en tres pasos:
    1. Desapila `c` de `pila_condiciones` y la dirección de retorno `pos_inicio` de `pila_saltos`.
    2. Genera un salto incondicional hacia el inicio de la condición (`crear_terceto("BI", "[T_inicio]", "-")`) para reevaluar la condición del `mientras`.
    3. Rellena todos los saltos falsos pendientes (`backpatch(c.falselist, indice_terceto)`) con el terceto siguiente al `BI`, dirigiendo la salida fuera del bucle cuando la condición deje de cumplirse.

#### 4.8 Sentencias de Salida y Retorno (`mostrar` / `retornar`)

- **Sentencia `mostrar` (`argumentos_mostrar` / `lista_ids_mostrar`):**
  - **Variante Cadena (`argumentos_mostrar: LIT_CADENA`):** Genera `crear_terceto("MOSTRAR_STR", $1, "-")`.
  - **Variante Variable (`argumentos_mostrar: ID` / `lista_ids_mostrar: ID | lista_ids_mostrar COMA ID`):** Verifica que la variable esté declarada y genera `crear_terceto("MOSTRAR_ID", ID, "-")`.
  - **Variante Mixta (`argumentos_mostrar: LIT_CADENA {A1} COMA lista_ids_mostrar`):** En `{A1}` emite primero `crear_terceto("MOSTRAR_STR", $1, "-")` y luego cada identificador de `lista_ids_mostrar` emite su propio terceto `MOSTRAR_ID` en orden de aparición.

- **Sentencia `retornar` (`retornar: PR_RETORNAR PAR_IZQ valor_retorno PAR_DER PUNTO_COMA`):**
  - **Acción:** La regla auxiliar `valor_retorno` (`ID | LIT_RACIONAL`) propaga el valor en `$$`, y `retornar` genera `crear_terceto("RETORNAR", $3, "-")`.

#### 4.9 Estructura del Programa, Declaración y Llamado de Funciones

En este lenguaje las funciones no reciben parámetros y el bloque `principal()` constituye el punto de entrada obligatorio del programa.

- **Inicio del programa y bloque `principal` (`programa: declaraciones {A1} funciones PR_PRINCIPAL PAR_IZQ PAR_DER {A2} bloque`):**
  - **En `{A1}` (tras las declaraciones globales):** Genera `crear_terceto("IR_PRINCIPAL", "-", "-")` para saltar por encima de las definiciones de funciones auxiliares.
  - **En `{A2}` (al abrir `principal()`):** Establece `funcion_actual = "principal"` y genera la marca de entrada mediante `crear_terceto("PRINCIPAL", "-", "-")`.

- **Definición de función (`funcion: PR_DEF ID PAR_IZQ PAR_DER {A1} bloque {A2}`):**
  - **En `{A1}`:** Obtiene el nombre de la función mediante `$2` (que representa el identificador `ID`), lo guarda en `funcion_actual` y genera la marca de inicio `crear_terceto("FUNC_INICIO", $2, "-")`.
  - **En `{A2}`:** Genera la marca de cierre `crear_terceto("FUNC_FIN", $2, "-")`, utilizando nuevamente `$2` para identificar la función, y restablece `funcion_actual = ""`.

- **Llamado como sentencia (`llamada_funcion: ID PAR_IZQ PAR_DER PUNTO_COMA`):**
  - **Acción:** Obtiene el nombre de la función mediante `$1` (que representa `ID`), verifica que no sea una llamada recursiva (`$1 != funcion_actual`) y genera `crear_terceto("CALL", $1, "-")`.

- **Llamado dentro de una expresión (`factor: ID PAR_IZQ PAR_DER`):**
  - **Acción:** Verifica que no haya recursividad (`$1 != funcion_actual`), genera el terceto `crear_terceto("CALL", $1, "-")` y guarda su referencia en `$$` para utilizar el valor devuelto por la función en una operación.



## 5. Criterios de aceptación

Para considerar que esta etapa funciona correctamente, se deben cumplir las siguientes condiciones:

1. **Orden correcto de las operaciones:** los tercetos deben generarse respetando la prioridad de los operadores. Por ejemplo, en `x = a + b * c`, primero debe generarse la multiplicación y después la suma.

2. **Formato del archivo:** cuando el análisis del programa finaliza correctamente, se debe generar el archivo `tercetos.txt`, con cada terceto en una línea y siguiendo el formato `[T_] (op, op1, op2)`.

3. **Saltos correctamente resueltos:** las instrucciones `BF` y `BI` deben tener indicado un destino válido dentro de la tabla de tercetos.

4. **Control de errores:** si el programa contiene errores léxicos o sintácticos (`error_lexico != 0` o `error_sintactico != 0`), no se debe generar ni exportar el archivo de tercetos.

