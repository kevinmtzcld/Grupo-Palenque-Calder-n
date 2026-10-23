Markdown# Spec — 04-analizador-sintactico

## Objetivo
Especificar e implementar el **Analizador Sintáctico (Parser)** para el lenguaje **Razio (Grupo C)** utilizando la herramienta **Bison**. Esta etapa tiene como propósito validar la corrección gramatical de las sentencias del programa fuente y servir de puente para la generación de código intermedio en formato de **Tercetos**.

---

## Alcance
El analizador sintáctico procesará la secuencia de tokens provista por el analizador léxico (`yylex()`) y reconocerá las siguientes estructuras del lenguaje Razio:
* Declaraciones globales y locales de variables racionales (`PR_RACIONAL`).
* Definición de funciones (`def`) con listas de parámetros e instrucciones de retorno (`PR_RETORNAR`).
* Bloque del programa principal (`principal`).
* Sentencias de asignación (`OP_ASIG`) y evaluación de expresiones aritméticas compuestas.
* Estructuras de control de flujo condicionales (`si` / `sino`) y repetitivas (`mientras`).
* Sentencias de salida estándar (`mostrar`).

---

## Entradas / Salidas

### Entradas
* **Tira de Tokens:** Secuencia de componentes léxicos leídos mediante llamadas iterativas a `yylex()` desde `alexico.c`.
* **Atributos Léxicos:** Información almacenada en la Tabla de Símbolos (`tabla_simbolos`).

### Salidas
* **Reporte de Análisis:** Indicación de finalización exitosa del análisis sintáctico o emisión de mensajes de error especificando el número de línea exacto.
* **Tabla de Tercetos (`out/tercetos.txt`):** Archivo plano con la representación del código intermedio generado durante las reducciones sintácticas.

---

## Reglas y decisiones de diseño

### 1. Definición Gramatical en BNF (Gramática de Razio)

```text
<programa>              ::= <declaraciones> <funciones> PR_PRINCIPAL PAR_IZQ PAR_DER LLAVE_IZQ <sentencias> LLAVE_DER

<declaraciones>         ::= <declaraciones> <declaracion>
                          | lambda

<declaracion>           ::= PR_RACIONAL <lista_variables> PUNTO_COMA

<lista_variables>       ::= <lista_variables> COMA ID
                          | ID

<funciones>             ::= <funciones> <funcion>
                          | lambda

<funcion>               ::= PR_DEF ID PAR_IZQ <parametros> PAR_DER LLAVE_IZQ <sentencias> LLAVE_DER

<parametros>            ::= PR_RACIONAL ID <lista_parametros>
                          | lambda

<lista_parametros>      ::= <lista_parametros> COMA PR_RACIONAL ID
                          | lambda

<sentencias>            ::= <sentencias> <sentencia>
                          | lambda

<sentencia>             ::= <asignacion>
                          | <est_seleccion>
                          | <est_iteracion>
                          | <salida>
                          | <retorno>

<asignacion>            ::= ID OP_ASIG <expresion> PUNTO_COMA

<est_seleccion>         ::= PR_SI PAR_IZQ <condicion> PAR_DER LLAVE_IZQ <sentencias> LLAVE_DER <bloque_sino>

<bloque_sino>           ::= PR_SINO LLAVE_IZQ <sentencias> LLAVE_DER
                          | lambda

<est_iteracion>         ::= PR_MIENTRAS PAR_IZQ <condicion> PAR_DER LLAVE_IZQ <sentencias> LLAVE_DER

<salida>                ::= PR_MOSTRAR PAR_IZQ <expresion_cadena> PAR_DER PUNTO_COMA

<expresion_cadena>      ::= LIT_CADENA
                          | <expresion>

<retorno>               ::= PR_RETORNAR <expresion> PUNTO_COMA

<condicion>             ::= <expresion> <comparador> <expresion>
                          | <condicion> OP_AND <condicion>
                          | <condicion> OP_OR <condicion>

<comparador>            ::= COMP_MAYOR | COMP_MENOR | COMP_MAYOR_IGUAL 
                          | COMP_MENOR_IGUAL | COMP_IGUAL | COMP_DISTINTO

<expresion>             ::= <expresion> OP_SUMA <termino>
                          | <expresion> OP_RESTA <termino>
                          | <termino>

<termino>               ::= <termino> OP_MULT <factor>
                          | <termino> OP_DIV <factor>
                          | <factor>

<factor>                ::= ID
                          | LIT_RACIONAL
                          | PAR_IZQ <expresion> PAR_DER
                          | <llamada_funcion>

<llamada_funcion>       ::= ID PAR_IZQ <argumentos> PAR_DER

<argumentos>            ::= <expresion> <lista_argumentos>
                          | lambda

<lista_argumentos>      ::= <lista_argumentos> COMA <expresion>
                          | lambda
2. Tablas de Precedencia y AsociatividadPara eliminar conflictos de tipo Shift/Reduce en Bison, se establece la jerarquía formal de operadores (de menor a mayor prioridad):   Fragmento de código%left OP_OR
%left OP_AND
%left COMP_IGUAL COMP_DISTINTO
%left COMP_MAYOR COMP_MENOR COMP_MAYOR_IGUAL COMP_MENOR_IGUAL
%left OP_SUMA OP_RESTA
%left OP_MULT OP_DIV
3. Representación de Código Intermedio (Tercetos)Estructura del Terceto: (Operador, Operando 1, Operando 2).   Identificación de Operandos:Puntero o índice hacia la Tabla de Símbolos (ID, LIT_RACIONAL, LIT_CADENA).Referencia cruzada a otro terceto previo mediante su número índice (ejemplo: [2])[cite: 1].Estructuras de Control (Backpatching / Relleno de Saltos):Para el si y mientras, se generan tercetos de salto condicional (BF - Branch if False) e incondicional (BI - Branch Inconditional) dejando la dirección de destino vacía[cite: 1].La dirección de salto se completa (rellena) mediante la pila semántica de Bison al momento de reconocer la reducción del fin del bloque (sino o fin de mientras)[cite: 1].4. Recuperación de Errores SintácticosSe hace uso del token reservado error de Bison en las reglas de declaración, asignación y estructuras de control.Ante un error sintáctico, el parser descarta tokens hasta encontrar el delimitador de cierre (como ; o }) para limpiar la pila de estados e informar el error sin detener abruptamente la compilación.
