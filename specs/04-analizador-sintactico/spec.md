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

La precedencia de los operadores aritméticos está resuelta directamente a nivel gramatical mediante las capas `<expresion>`, `<termino>` y `<factor>`. Para los operadores relacionales y lógicos, se definen reglas de asociatividad a izquierda en Bison para eliminar ambigüedades en la tabla de estados:

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
    declaraciones funciones PR_PRINCIPAL PAR_IZQ PAR_DER LLAVE_IZQ sentencias LLAVE_DER
;

declaraciones:
    declaraciones declaracion
  | /* vacio */
;

declaracion:
    PR_RACIONAL lista_variables PUNTO_COMA
;

lista_variables:
    lista_variables COMA ID
  | ID
;

funciones:
    funciones funcion
  | /* vacio */
;

funcion:
    PR_DEF ID PAR_IZQ parametros PAR_DER LLAVE_IZQ sentencias_func LLAVE_DER
;

parametros:
    PR_RACIONAL ID lista_parametros
  | /* vacio */
;

lista_parametros:
    lista_parametros COMA PR_RACIONAL ID
  | /* vacio */
;

sentencias_func:
    sentencias_func sentencia_func
  | /* vacio */
;

sentencia_func:
    sentencia
  | retorno
;

sentencias:
    sentencias sentencia
  | /* vacio */
;

sentencia:
    asignacion
  | est_seleccion
  | est_iteracion
  | salida
  | error PUNTO_COMA { yyerrok; }
;

asignacion:
    ID OP_ASIG expresion PUNTO_COMA
;

est_seleccion:
    PR_SI PAR_IZQ condicion PAR_DER LLAVE_IZQ sentencias LLAVE_DER bloque_sino
;

bloque_sino:
    PR_SINO LLAVE_IZQ sentencias LLAVE_DER
  | /* vacio */
;

est_iteracion:
    PR_MIENTRAS PAR_IZQ condicion PAR_DER LLAVE_IZQ sentencias LLAVE_DER
;

salida:
    PR_MOSTRAR PAR_IZQ expresion_cadena PAR_DER PUNTO_COMA
;

expresion_cadena:
    LIT_CADENA
  | expresion
;

retorno:
    PR_RETORNAR expresion PUNTO_COMA
;

condicion:
    expresion comparador expresion
  | condicion OP_AND condicion
  | condicion OP_OR condicion
;

comparador:
    COMP_MAYOR
  | COMP_MENOR
  | COMP_MAYOR_IGUAL
  | COMP_MENOR_IGUAL
  | COMP_IGUAL
  | COMP_DISTINTO
;

expresion:
    expresion OP_SUMA termino
  | expresion OP_RESTA termino
  | termino
;

termino:
    termino OP_MULT factor
  | termino OP_DIV factor
  | factor
;

factor:
    ID
  | LIT_RACIONAL
  | PAR_IZQ expresion PAR_DER
  | llamada_funcion
;

llamada_funcion:
    ID PAR_IZQ argumentos PAR_DER
;

argumentos:
    expresion lista_argumentos
  | /* vacio */
;

lista_argumentos:
    lista_argumentos COMA expresion
  | /* vacio */
;

%%
```

### 4. Definición y Manejo de Errores Sintácticos

Se contempla la captura y recuperación frente a errores de estructura mediante el token reservado `error` en la regla `sentencia` (`error PUNTO_COMA { yyerrok; }`). Ante una sentencia mal construida (por ejemplo, omitir un operando o un paréntesis), Bison descarta tokens hasta encontrar el delimitador de fin de sentencia (`;`), emite un reporte indicando el número de línea (`num_linea`) y reanuda el análisis sintáctico de las sentencias subsiguientes sin abortar el compilador.

---

## Criterios de aceptación

Esta sección establece las condiciones objetivas y verificables que debe cumplir el analizador sintáctico para dar por aprobada su implementación. Sirve como lista de verificación para validar la implementación del parser en Bison:

* **Validación de Programas Correctos:** Reconoce y acepta exitosamente los programas fuente que cumplen de forma estricta con la gramática definida para el lenguaje Razio, retornando el valor 0 desde yyparse().
* **Detección y Reporte de Errores Sintácticos:** Rechaza programas que violen las reglas gramaticales, emitiendo un mensaje descriptivo por consola que informa el número de línea exacto (num_linea) y el token donde ocurrió la falla.   
* **Compilación Sin Conflictos:** El archivo de gramática (sintactico.y) debe compilar sin generar ningún conflicto de ambigüedad del tipo Shift/Reduce ni Reduce/Reduce.
* **Respeto de Precedencia y Asociatividad:** Respeta la prioridad de los operadores aritméticos mediante la jerarquía gramatical (<expresion>, <termino>, <factor>) y resuelve la asociatividad a izquierda de los operadores relacionales y lógicos mediante directivas %left.
* **Recuperación de Errores por Pánico:** Ante una sentencia mal formada, descarta tokens hasta el siguiente delimitador ; mediante la regla error PUNTO_COMA { yyerrok; }, logrando reanudar el análisis de las sentencias subsiguientes sin abortar la ejecución del compilador de forma abrupta.   

* **Finalización Limpia:** Completa el análisis sintáctico (retornando valor `0` desde `yyparse()`) al procesar un programa fuente totalmente válido.
