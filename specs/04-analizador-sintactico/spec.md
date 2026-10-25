# Spec — 04-analizador-sintactico

## Objetivo

Esta etapa tiene como propósito validar la corrección gramatical de las sentencias del programa fuente a partir de la secuencia de tokens suministrada por el analizador léxico.

---
## Alcance

Esta sección define los límites de responsabilidad del analizador sintáctico y enumera el conjunto de construcciones del lenguaje **Razio** que el parser está diseñado para reconocer y validar. 

El módulo procesará de manera iterativa los tokens provistos por la función `yylex()` desde `alexico.c` y verificará el cumplimiento estricto de la estructura del programa, abarcando las siguientes reglas gramaticales:

* **Estructura General del Programa:** Validación del orden obligatorio de un archivo fuente en Razio (declaraciones globales al inicio, seguidas de la definición de funciones y el bloque `principal () { ... }` obligatorio).
* **Declaraciones de Variables Globales:** Reconocimiento de variables de tipo racional (`PR_RACIONAL`) ubicadas exclusivamente al comienzo del programa.
* **Definición y Modularización de Funciones:** Declaración de funciones mediante la palabra reservada `def` (`PR_DEF`), paso de parámetros de tipo racional, su bloque de sentencias interno y la sentencia de retorno (`PR_RETORNAR`).
* **Sentencias y Expresiones Permitidas:** Validación sintáctica de asignaciones de expresiones aritméticas compuestas, estructuras de control condicionales (`si` / `sino`), bucles de iteración (`mientras`) y llamadas a la función de salida por pantalla (`mostrar`)
---


---
## Entradas / Salidas
Esta sección establece el contrato de interfaz del analizador sintáctico, definiendo los datos que requiere del módulo léxico para operar y los resultados o reportes que emite tras la validación gramatical.

### Entradas
* **Tira de Tokens (Flujo Léxico):** Secuencia de códigos numéricos de componentes léxicos entregados a demanda (on-demand) e iterativamente por la función `yylex()` desde `alexico.c`.
* **Atributos Léxicos y Contexto:** Información adicional asociada a los tokens (como el lexema exacto o el valor de la constante), recuperada mediante la variable global `buffer` o mediante consultas directas a la Tabla de Símbolos (`tabla_simbolos`).

### Salidas
* **Reporte de Análisis:** 
  * **En caso de éxito:** Notificación en consola confirmando que el programa fuente cumple de forma estricta con la gramática del lenguaje Razio.
  * **En caso de error:** Mensaje(s) descriptivo(s) de error sintáctico emitido(s) por consola, indicando el número de línea exacto (`num_linea`) y el token donde se detectó el fallo de estructura.
---

## Reglas y decisiones de diseño

### 1. Declaración de Tokens para Bison (`%token`)

Definición de las constantes de componentes léxicos que el parser recibe desde `yylex()`:

```yacc
%token PR_DEF PR_PRINCIPAL PR_RACIONAL PR_SI PR_SINO PR_MIENTRAS PR_MOSTRAR PR_RETORNAR
%token ID LIT_RACIONAL LIT_CADENA
%token OP_ASIG OP_SUMA OP_RESTA OP_MULT OP_DIV
%token COMP_MAYOR COMP_MENOR COMP_MAYOR_IGUAL COMP_MENOR_IGUAL COMP_IGUAL COMP_DISTINTO
%token OP_AND OP_OR
%token PAR_IZQ PAR_DER LLAVE_IZQ LLAVE_DER PUNTO_COMA COMA
```

### 2. Precedencia y Asociatividad de Operadores

La precedencia de los operadores aritméticos está resuelta directamente a nivel gramatical mediante las capas `<expresion>`, `<termino>` y `<factor>`, ej: una * se resuelve antes que una suma porque * esta en nivel inferior y debe resolverse antes, sino la suma no se puede realizar.
Directivas de Bison (Lógicas y Relacionales): Para los operadores lógicos y de comparación, la precedencia y la asociatividad a izquierda se definen de menor (inicio) a mayor (fin) jerarquía mediante la directiva %left:
-prioridad menor(1ro) a mayor (4to):

```yacc
%left OP_OR
%left OP_AND
%left COMP_IGUAL COMP_DISTINTO
%left COMP_MAYOR COMP_MENOR COMP_MAYOR_IGUAL COMP_MENOR_IGUAL
```

### 3. Gramática de Razio traducida a Sintaxis Bison

```yacc

%%

programa:
    declaraciones funciones PR_PRINCIPAL PAR_IZQ PAR_DER bloque
;

declaraciones:
    declaracion
  | /* vacio */
;

declaracion:
    PR_RACIONAL lista_variables PUNTO_COMA
;

lista_variables:
    variable
  | lista_variables COMA variable
;

variable:
    ID
  | ID OP_ASIG LIT_RACIONAL
;

funciones:
    /* vacio */
  | funciones funcion
;

funcion:
    PR_DEF ID PAR_IZQ PAR_DER bloque
;

bloque:
    LLAVE_IZQ sentencias LLAVE_DER
;

sentencias:
    /* vacio */
  | sentencias sentencia
;

sentencia:
    asignacion
  | si
  | mientras
  | mostrar
  | retornar
  | llamada_funcion
  | error PUNTO_COMA { yyerrok; }
;

asignacion:
    ID OP_ASIG expresion PUNTO_COMA
;

expresion:
    termino
  | expresion OP_SUMA termino
  | expresion OP_RESTA termino
;

termino:
    factor
  | termino OP_MULT factor
  | termino OP_DIV factor
;

factor:
    LIT_RACIONAL
  | ID
  | PAR_IZQ expresion PAR_DER
  | ID PAR_IZQ PAR_DER
;

si:
    PR_SI PAR_IZQ condicion PAR_DER bloque
  | PR_SI PAR_IZQ condicion PAR_DER bloque PR_SINO bloque
;

mientras:
    PR_MIENTRAS PAR_IZQ condicion PAR_DER bloque
;

mostrar:
    PR_MOSTRAR PAR_IZQ argumentos_mostrar PAR_DER PUNTO_COMA
;

argumentos_mostrar:
    ID
  | LIT_CADENA
  | LIT_CADENA COMA lista_ids_mostrar
;

lista_ids_mostrar:
    ID
  | lista_ids_mostrar COMA ID
;

retornar:
    PR_RETORNAR PAR_IZQ valor_retorno PAR_DER PUNTO_COMA
;

valor_retorno:
    ID
  | LIT_RACIONAL
;

llamada_funcion:
    ID PAR_IZQ PAR_DER PUNTO_COMA
;

condicion:
    cond_or
;

cond_or:
    cond_and
  | cond_or OP_OR cond_and
;

cond_and:
    cond_factor
  | cond_and OP_AND cond_factor
;

cond_factor:
    PAR_IZQ condicion PAR_DER
  | comparacion
;

comparacion:
    expresion operador_comparacion expresion
;

operador_comparacion:
    COMP_MAYOR
  | COMP_MENOR
  | COMP_MAYOR_IGUAL
  | COMP_MENOR_IGUAL
  | COMP_IGUAL
  | COMP_DISTINTO
;

%%


```
### 4. Definición y Manejo de Errores Sintácticos

#### Clasificación de Errores Sintácticos Contemplados
El analizador sintáctico identificará y reportará, entre otros, los siguientes tipos de fallos de estructura:

1. **Omisión de Delimitadores Obligatorios:** Falta del punto y coma (`;`) al final de asignaciones, retornos o llamadas a función, o falta de llaves `{}` / paréntesis `()` en bloques de control (`si`, `mientras`).
   * *Ejemplo:* `x = a + 5  // Falta ';'`
2. **Expresiones Incompletas o Mal Formadas:** Operadores aritméticos, relacionales o lógicos que carecen de uno de sus operandos.
   * *Ejemplo:* `total = (a + ) * 2;`
3. **Desbalanceo de Paréntesis/Llaves:** Apertura de estructuras condicionales o expresiones sin su correspondiente cierre.
   * *Ejemplo:* `si (x > 10 { mostrar("error"); }`
4. **Violación de Secuencia Sintáctica:** Ubicación de palabras reservadas o sentencias en lugares no permitidos según la gramática.
   * *Ejemplo:* Uso de `retornar` fuera de una función `def`.

#### Mecanismo de Recuperación de Errores
Se implementa un esquema de **recuperación por pánico** mediante la regla `sentencia -> error PUNTO_COMA { yyerrok; }`.

Ante cualquier sentencia mal construida, Bison reacciona de la siguiente manera:
* **Descarte de tokens:** Elimina los tokens defectuosos hasta hallar el delimitador de fin de sentencia (`;`).
* **Reporte claro:** Emite un mensaje de error por consola especificando el número de línea exacto (`num_linea`) y el token del fallo.
* **Reanudación:** Restablece el estado del parser vía `yyerrok` y continúa analizando el resto del programa sin abortar la compilación abruptamente.
---

## Criterios de aceptación

Esta sección establece las condiciones objetivas y verificables que debe cumplir el analizador sintáctico para dar por aprobada su implementación. Sirve como lista de verificación para validar la implementación del parser en Bison:

* **Validación de Programas Correctos:** Reconoce y acepta exitosamente los programas fuente que cumplen de forma estricta con la gramática definida para el lenguaje Razio, retornando el valor 0 desde yyparse().
* **Detección y Reporte de Errores Sintácticos:** Rechaza programas que violen las reglas gramaticales, emitiendo un mensaje descriptivo por consola que informa el número de línea exacto (num_linea) y el token donde ocurrió la falla.   
* **Compilación Sin Conflictos:** El archivo de gramática (sintactico.y) debe compilar sin generar ningún conflicto de ambigüedad del tipo Shift/Reduce ni Reduce/Reduce.
* **Respeto de Precedencia y Asociatividad:** Respeta la prioridad de los operadores aritméticos mediante la jerarquía gramatical (<expresion>, <termino>, <factor>) y resuelve la asociatividad a izquierda de los operadores relacionales y lógicos mediante directivas %left.
* **Recuperación de Errores por Pánico:** Ante una sentencia mal formada, descarta tokens hasta el siguiente delimitador ; mediante la regla error PUNTO_COMA { yyerrok; }, logrando reanudar el análisis de las sentencias subsiguientes sin abortar la ejecución del compilador de forma abrupta.   

* **Finalización Limpia:** Completa el análisis sintáctico (retornando valor `0` desde `yyparse()`) al procesar un programa fuente totalmente válido.
