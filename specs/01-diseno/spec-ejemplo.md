## 1. DECISIONES GLOBALES

| # | Decisión | Valor |
| :-: | :--- | :--- |
| D1 | Tipo de datos | Un solo tipo: racional con signo, 32 bits |
| D2 | Rango | −2.147.483.648 a 2.147.483.647 |
| D3 | Declaración | Obligatoria, debe preceder al primer uso |
| D4 | Alcance | Único y global: no hay variables locales |
| D5 | Sensibilidad a mayúsculas | Sí. Total y total son variables distintas |
| D6 | Longitud máxima de identificador | 20 caracteres; más largo se trunca con advertencia |
| D7 | Comentarios | Multilínea, delimitados por /* y */ |
| D8 | Resultado de una comparación | No es un valor. Solo aparece en condiciones |
| D9 | División por cero | Error en compilación para literales (ej: 3/0) y en ejecución para expresiones |
| D10 | Plataforma destino | x86-64, NASM sobre Linux |




## 2.Alfabeto

Σ = L ∪ D ∪ O ∪ S ∪ B

| Clase | Caracteres |
| :---: | :--- |
| *L* | `a`–`z`, `A`–`Z`|
| *D* | 0 – 9 |
| *O* | +, -, *, /, =, <, > !|
| *S* | (, ), {, }, ;, :, ,, " |
| *B* | espacio, tabulación (\t), salto de línea (\n)|
| *OTRO* | cualquier otro carácter → error léxico |

**Cualquier carácter presente en el archivo .txt que no pertenezca a esta unión de conjuntos (por ejemplo: @, #, ~, ?, _, etc.) 
será rechazado por el analizador léxico, y generará un error léxico con el número de línea correspondiente**

## 3.PALABRAS RESERVADAS
def · principal  · racional · si · sino · mientras · mostrar · retornar

## 4. Tabla de tokens

|  Código | Token              | Lexema                          |
| :-----: | :----------------- | :------------------------------ |
| *257* | PR_DEF           | def                           |
| *258* | PR_PRINCIPAL     | principal                     |
| *259* | PR_RACIONAL      | racional                      |
| *260* | PR_SI            | si                            |
| *261* | PR_SINO          | sino                          |
| *262* | PR_MIENTRAS      | mientras                      |
| *263* | PR_MOSTRAR       | mostrar                       |
| *264* | PR_RETORNAR      | retornar                      |
| *265* | ID               | Identificador (a, acum)     |
| *266* | LIT_RACIONAL     | Constante racional (3/4, 5) |
| *267* | LIT_CADENA       | Cadena de caracteres ("Hola") |
| *268* | OP_ASIG          | =                             |
| *269* | OP_SUMA          | +                             |
| *270* | OP_RESTA         | -                             |
| *271* | OP_MULT          | *                             |
| *272* | OP_DIV           | /                             |
| *273* | COMP_MAYOR       | >                             |
| *274* | COMP_MENOR       | <                             |
| *275* | COMP_MAYOR_IGUAL | >=                            |
| *276* | COMP_MENOR_IGUAL | <=                            |
| *277* | COMP_IGUAL       | ==                            |
| *278* | COMP_DISTINTO    | !=                            |
| *279* | OP_AND           | and                           |
| *280* | OP_OR            | or                            |
| *281* | PAR_IZQ          | (                             |
| *282* | PAR_DER          | )                             |
| *283* | LLAVE_IZQ        | {                             |
| *284* | LLAVE_DER        | }                             |
| *285* | PUNTO_COMA       | ;                             |
| *286* | COMA             | ,                             |


## 5. Estructura del programa

Un programa en **Razio** se compone de una sección inicial de declaraciones globales de variables seguida por la definición de una o más funciones en memoria estática.

- **Punto de Entrada:** La ejecución comienza obligatoriamente en la función con el nombre reservado `principal`.
- **Funciones:** No reciben parámetros y operan sobre el espacio de nombres global. Pueden retornar un valor de tipo racional.
- **Restricción de Recursión:** Está estrictamente prohibido que una función se llame a sí misma de manera directa; esta condición es detectada y reportada como error semántico durante la compilación.


## 6. GRAMÁTICA

<programa>::= <declaracion><funciones> PR_PRINCIPAL PAR_IZQ PAR_DER <bloque>

<declaracion> ::= PR_RACIONAL <lista_variables> PUNTO_COMA
<lista_variables> ::= <variable> | <lista_variables> COMA <variable>
<variable> ::= ID | ID OP_ASIG LIT_RACIONAL


<funciones> ::= lambda | <funciones> <funcion>
<funcion> ::= PR_DEF ID PAR_IZQ PAR_DER <bloque>
<bloque> ::= LLAVE_IZQ <sentencias> LLAVE_DER

<sentencias> ::= lambda | <sentencias> <sentencia>
<sentencia> ::= <asignacion> | <si>| <mientras>| <mostrar>| <retornar> | <llamada_funcion>

<asignacion> ::= ID OP_ASIG <expresion> PUNTO_COMA
<expresion> ::= <termino> | <expresion> OP_SUMA <termino> | <expresion> OP_RESTA <termino>
<termino> ::= <factor> | <termino> OP_MULT <factor> | <termino> OP_DIV <factor>
<factor> ::= LIT_RACIONAL | ID | PAR_IZQ <expresion> PAR_DER | ID PAR_IZQ PAR_DER

<si> ::= PR_SI PAR_IZQ <condicion> PAR_DER <bloque> PR_SINO <bloque>
<mientras> ::= PR_MIENTRAS PAR_IZQ <condicion> PAR_DER <bloque>

<mostrar> ::= PR_MOSTRAR PAR_IZQ <argumentos_mostrar> PAR_DER PUNTO_COMA
<argumentos_mostrar> ::= ID | LIT_CADENA  |  LIT_CADENA COMA <lista_ids_mostrar>
<lista_ids_mostrar> ::= ID | <lista_ids_mostrar> COMA ID

<retornar> ::= PR_RETORNAR PAR_IZQ <valor_retorno> PAR_DER PUNTO_COMA
<valor_retorno> ::= ID | LIT_RACIONAL

<llamada_funcion> ::= ID PAR_IZQ PAR_DER PUNTO_COMA



<condicion> ::= <cond_or>

<cond_or> ::= <cond_and> | <cond_or> OP_OR <cond_and>

<cond_and> ::= <cond_factor> | <cond_and> OP_AND <cond_factor>

<cond_factor> ::= PAR_IZQ <condicion> PAR_DER | <comparacion>

<comparacion> ::= <expresion> <operador_comparacion> <expresion>

<operador_comparacion> ::= COMP_MAYOR | COMP_MENOR | COMP_MAYOR_IGUAL | COMP_MENOR_IGUAL | COMP_IGUAL | COMP_DISTINTO


**Notas sobre la gramática**

* Recursión a izquierda en `<expresion>`, `<termino>`, `<sentencias>`, `<funciones>` y `<lista_variables>`: es la forma que prefiere una herramienta YACC.
* La precedencia queda resuelta por la estructura en tres niveles (`expresion` → `termino` → `factor`), no por declaraciones de precedencia.
* El `sino` colgante no existe como problema: `<bloque>` siempre lleva llaves.
* Soporte de operadores lógicos: `<condicion>` permite `and`, `or` y agrupamiento por paréntesis, según lo requerido.

## 7. Semántica

| Regla | Definición |
|---|---|
| R1 | Usar un `ID` no declarado es error semántico |
| R2 | Declarar dos veces el mismo `ID` es error semántico |
| R3 | Toda variable se inicializa en `0/1` antes de la primera sentencia |
| R4 | Las constantes se registran en la tabla de símbolos con nombre `_valor` |
| R5 | Una constante fuera del rango D2 es error semántico, con línea |
| R6 | Intentar definir un literal con denominador `0` (ej: `3/0`) es error semántico |
| R7 | La llamada a una función que se invoque a sí misma (**recursión**) es error semántico |
| R8 | `mostrar` imprime la fracción simplificada por MCD seguida de un salto de línea |


## 8. Responsabilidad de cada error

| Código | Descripción | Fase que lo detecta |
|---|---|---|
| E1 | Carácter fuera del alfabeto | Léxico |
| E2 | Constante o literal mal formado | Léxico |
| E3 | Constante o literal entero fuera del rango D2 (32 bits) | Léxico / Semántico |
| E4 | Sentencia o estructura mal formada | Sintáctico |
| E5 | Variable no declarada previa a su uso (R1) | Semántico (sobre Tabla de Símbolos) |
| E6 | Variable redeclarada en el ámbito global (R2) | Semántico (sobre Tabla de Símbolos) |
| E7 | Denominador en cero en constante literal (ej: `3/0`) | Semántico (en Compilación) |
| E8 | Llamada a función que invoque a sí misma (Recursión prohibida) | Semántico (en Compilación) |
| E9 | División por cero durante el cálculo de expresiones | Ejecución (código Assembler emitido) |
| E10 | Desbordamiento (*Overflow*) en operaciones con 32 bits | Ejecución (verificación preventiva previa) |

*Ninguno de los errores detectados en tiempo de compilación (E1 a E8) aborta el proceso de forma inmediata: se registran con su correspondiente número de línea y se continúa con el análisis para reportar la mayor cantidad posible de errores en una sola corrida.*




## 9.PROGRAMA DE EJEMPLO

racional x,y,z=4/3,calculo,resultado,c=0;
/*INICIO DE VARIABLES
COMENTARIO*/

def funX()
{
    mostrar("Calculando");
    calculo=x+y;
    retornar (calculo);
}

principal()
{
    x=9/2;
    y=9/8;

    resultado=funX();

    si((x>y) and (c<3))
    {
        mostrar("x es mayor a y");

        mientras((c<3) or (x<y))
        {
            mostrar("valor de c:",c);
            c=c+1;
        }
    }
    sino
    {
        si(x<y)
        {
            mostrar("Y es mayor a x");
        }
        sino
        {
            mostrar("X e y son iguales");
        }
    }
}




## 10. Fuera de alcance

Se deja constancia de lo que **Razio** no incluye en su alcance, para que ninguna fase del compilador lo asuma:

* **Tipos de datos adicionales:** Números de punto flotante (*reales*), caracteres sueltos (`char`), valores booleanos y arreglos/vectores.
* **Manejo de funciones:** Paso de parámetros o argumentos a funciones, retorno de múltiples valores, variables locales por función y **recursión** (tanto directa como indirecta).
* **Ámbitos locales:** No existen variables locales ni ocultamiento de variables (*shadowing*); todas las variables son globales y comparten el mismo espacio de memoria estática.
* **Operadores adicionales:** Operador de negación lógica (`not`) e incrementos/decrementos unarios (`++`, `--`).
* **Entrada de datos:** Sentencias de lectura por teclado o consola (los datos ingresan únicamente mediante inicialización y asignación en el código fuente).



### AUTÓMATA FINÍTO
<img width="1986" height="2259" alt="autom comp pc drawio" src="https://github.com/user-attachments/assets/bb5298cd-1c83-4068-a473-6628d1f27fb4" />



