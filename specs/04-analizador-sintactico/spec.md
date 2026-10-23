Markdown
# Spec — 04-analizador-sintactico

## Objetivo
Esta etapa tiene como propósito validar la corrección gramatical de las sentencias del programa fuente y servir de puente para la generación de código intermedio en formato de **Tercetos**.

---

## Alcance
El analizador sintáctico procesará la secuencia de tokens enviada por la función `yylex()` desde `alexico.c` y verificará el cumplimiento estricto de la estructura gramatical del lenguaje Razio, contemplando:

* **Sección de Declaraciones Globales:** Reconocimiento de variables de tipo racional (`PR_RACIONAL`) declaradas exclusivamente al inicio del programa.
* **Definición de Funciones:** Declaraciones con `def` (`PR_DEF`), parámetros de tipo racional y su bloque de sentencias, con soporte para la sentencia de retorno (`PR_RETORNAR`).
* **Bloque Principal:** Estructura obligatoria del programa `principal () { ... }`.
* **Sentencias Permitidas:** Asignaciones de expresiones racionales, estructuras de selección (`si` / `sino`), iteración (`mientras`) y llamadas a la función de salida (`mostrar`)[cite: 1].

---


## Entradas / Salidas

### Entradas
* **Tira de Tokens:** Secuencia de códigos de componentes léxicos entregados de manera iterativa por la función `yylex()` desde `alexico.c`.
* **Atributos de Léxico:** Información complementaria (lexemas y valores) recuperada a través de la variable `buffer` o mediante la consulta a la Tabla de Símbolos (`tabla_simbolos`).

### Salidas
* **Reporte de Análisis:** Confirmación en consola de análisis sintáctico exitoso o emisión de mensajes de error sintáctico especificando el número de línea exacto (`num_linea`) donde se detectó el fallo.
---




---
## Reglas y decisiones de diseño

### 1. Declaración de Tokens para Bison (`%token`)
Definición de las constantes de componentes léxicos que el parser recibe desde `yylex()`:
---

```yacc
%token PR_DEF PR_PRINCIPAL PR_RACIONAL PR_SI PR_SINO PR_MIENTRAS PR_MOSTRAR PR_RETORNAR
%token ID LIT_RACIONAL LIT_CADENA
%token OP_ASIG OP_SUMA OP_RESTA OP_MULT OP_DIV
%token COMP_MAYOR COMP_MENOR COMP_MAYOR_IGUAL COMP_MENOR_IGUAL COMP_IGUAL COMP_DISTINTO
%token OP_AND OP_OR
%token PAR_IZQ PAR_DER LLAVE_IZQ LLAVE_DER PUNTO_COMA COMA
```



### 2.Precedencia y Asociatividad de Operadores
La precedencia de los operadores aritméticos está resuelta directamente a nivel gramatical mediante las capas <expresion>, <termino> y <factor>. Para los operadores relacionales y lógicos, se definen reglas de asociatividad a izquierda en Bison para eliminar ambigüedades en la tabla de estados:

```yacc
%left OP_OR
%left OP_AND
%left COMP_IGUAL COMP_DISTINTO
%left COMP_MAYOR COMP_MENOR COMP_MAYOR_IGUAL COMP_MENOR_IGUAL
```




### 3. Gramática de Razio traducida a Sintaxis Bison
```
Fragmento de código%%

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


#### 4.Definición y Manejo de Errores SintácticosSe contempla la captura y recuperación frente a errores de estructura mediante el token reservado error en la regla sentencia (error PUNTO_COMA { yyerrok; }).Ante una sentencia mal construida (por ejemplo, omitir un operando o un paréntesis), Bison descarta tokens hasta encontrar el delimitador de fin de sentencia (;), emite un reporte indicando el número de línea (num_linea) y reanuda el análisis sintáctico de las sentencias subsiguientes sin abortar el compilador.   













## Criterios de aceptación

* **Validación de Programas Correctos:** Reconoce y acepta exitosamente los programas fuente que cumplen de forma estricta con la gramática definida para el lenguaje Razio.
* **Detección y Reporte de Errores Sintácticos:** Rechaza programas con errores de sintaxis, emitiendo un mensaje de error por consola que informa el número de línea exacto (`num_linea`) donde ocurrió la falla[cite: 1].
* **Compilación Sin Conflictos:** El archivo de gramática procesado por Bison debe compilar limpiamente sin presentar ningún conflicto de tipo *Shift/Reduce* ni *Reduce/Reduce*.
* **Respeto de Precedencia y Asociatividad:** Resuelve correctamente la prioridad de operadores aritméticos (vía estructura gramatical de expresiones, términos y factores) y de operadores lógicos/relacionales (vía directivas `%left`).
* **Recuperación de Errores por Pánico:** Omitir una sentencia errónea mediante el token reservado `error` y el delimitador `;`, logrando reanudar el análisis de las sentencias siguientes sin abortar la compilación abruptamente[cite: 1].
* **Finalización Limpia:** Completa el análisis sintáctico (retornando valor `0` desde `yyparse()`) al procesar un programa fuente totalmente válido.
