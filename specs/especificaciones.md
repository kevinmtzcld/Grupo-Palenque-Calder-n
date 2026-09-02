## 1. Alfabeto

Σ = L ∪ D ∪ O ∪ R ∪ S ∪ B

| Clase | Caracteres |
| :---: | :--- |
| **L** | `a – z`, `A – Z` |
| **D** | `0 – 9` |
| **O** | `+`, `-`, `*`, `/`, `=`, `<`, `>` |
| **R** | `$` (utilizado para delimitar racionales) |
| **S** | `(`, `)`, `{`, `}`, `;`, `:`, `,`, `"` |
| **B** | espacio, tabulación (`\t`), salto de línea (`\n`)|
| **OTRO** | cualquier otro carácter → error léxico |

**Cualquier carácter presente en el archivo .txt que no pertenezca a esta unión de conjuntos (por ejemplo: @, #, ~, ?, _, etc.) 
será rechazado por el analizador léxico, y generará un error léxico con el número de línea correspondiente**

## 2.PALABRAS RESERVADAS
def · principal · entero · racional · cadena · si · sino · mientras · mostrar · retornar

## 3. Tabla de tokens

| Código | Token | Lexema |
| :---: | :--- | :--- |
| **257** | `PR_DEF` | `def` |
| **258** | `PR_RACIONAL` | `racional` |
| **259** | `PR_SI` | `si` |
| **260** | `PR_SINO` | `sino` |
| **261** | `PR_MIENTRAS` | `mientras` |
| **262** | `PR_MOSTRAR` | `mostrar` |
| **263** | `PR_PRINCIPAL` | `principal` |
| **264** | `ID` | Identificador (`a`, `acum`) |
| **265** | `LIT_RACIONAL` | Constante racional (`$3/4$`) |
| **266** | `LIT_ENTERO` | Constante entera (`5`) |
| **267** | `LIT_CADENA` | Cadena de texto (`"Resultado:"`) |
| **268** | `OP_ASIG` | `=` |
| **269** | `OP_SUMA` | `+` |
| **270** | `OP_RESTA` | `-` |
| **271** | `OP_MULT` | `*` |
| **272** | `OP_DIV` | `/` |
| **273** | `COMP_MAYOR` | `>` |
| **274** | `COMP_MENOR` | `<` |
| **275** | `COMP_MAYOR_IGUAL` | `>=` |
| **276** | `COMP_MENOR_IGUAL` | `<=` |
| **277** | `COMP_IGUAL` | `==` |
| **278** | `COMP_DISTINTO` | `!=` |
| **279** | `OP_AND` | `and` |
| **280** | `OP_OR` | `or` |
| — | Literales | `(`, `)`, `{`, `}`, `;`, `:`, `,` se devuelven como su propio carácter |





## 4.PROGRAMA DE EJEMPLO
```
def obtener_bono() {
    racional bono;
    bono = $1/2$;
    mostrar("CALCULANDO BONO...");
    retornar bono;
}

principal() {
    racional total;
    entero usuario_id;
    cadena mensaje;

    usuario_id = 101;
    mensaje = "PROCESANDO USUARIO:";

    mostrar(mensaje);
    mostrar(usuario_id);

    // LLAMADA A FUNCION AUXILIAR
    total = obtener_bono();

    mostrar("TOTAL CON BONO:");
    mostrar(total);
}

```




## 5. GRAMÁTICA
```
<programa>              ::= <lista_funciones> PR_PRINCIPAL '(' ')' <bloque>

<lista_funciones>      ::= <lista_funciones> <definicion_funcion>
                          | ε

<definicion_funcion>   ::= PR_DEF ID '(' ')' <bloque>

<bloque>                ::= '{' <lista_sentencias> '}'

<lista_sentencias>      ::= <lista_sentencias> <sentencia>
                          | <sentencia>

<sentencia>             ::= <sentencia_declaracion>
                          | <sentencia_asignacion>
                          | <sentencia_si>
                          | <sentencia_mientras>
                          | <sentencia_mostrar>
                          | <sentencia_retorno>
                          | <llamada_funcion> ';'

<sentencia_declaracion> ::= <tipo_dato> <lista_variables> ';'

<tipo_dato>             ::= PR_ENTERO
                          | PR_RACIONAL
                          | PR_CADENA

<lista_variables>       ::= <lista_variables> ',' ID
                          | ID

<sentencia_asignacion>  ::= ID OP_ASIG <expresion_matematica> ';'
                          | ID OP_ASIG LIT_CADENA ';'

<sentencia_si>          ::= PR_SI '(' <expresion_logica> ')' <bloque>
                          | PR_SI '(' <expresion_logica> ')' <bloque> PR_SINO <bloque>

<sentencia_mientras>    ::= PR_MIENTRAS '(' <expresion_logica> ')' <bloque>

<sentencia_mostrar>     ::= PR_MOSTRAR '(' <expresion_matematica> ')' ';'
                          | PR_MOSTRAR '(' LIT_CADENA ')' ';'

<sentencia_retorno>     ::= PR_RETORNAR <expresion_matematica> ';'

<llamada_funcion>       ::= ID '(' ')'

<expresion_logica>      ::= <expresion_logica> OP_OR <termino_logico>
                          | <termino_logico>

<termino_logico>        ::= <termino_logico> OP_AND <factor_logico>
                          | <factor_logico>

<factor_logico>         ::= <expresion_matematica> <operador_comparacion> <expresion_matematica>
                          | '(' <expresion_logica> ')'

<operador_comparacion>  ::= COMP_IGUAL 
                          | COMP_DISTINTO 
                          | COMP_MENOR 
                          | COMP_MAYOR 
                          | COMP_MENOR_IGUAL 
                          | COMP_MAYOR_IGUAL

<expresion_matematica>  ::= <expresion_matematica> OP_SUMA <termino_matematico>
                          | <expresion_matematica> OP_RESTA <termino_matematico>
                          | <termino_matematico>

<termino_matematico>    ::= <termino_matematico> OP_MULT <factor_matematico>
                          | <termino_matematico> OP_DIV <factor_matematico>
                          | <factor_matematico>

<factor_matematico>     ::= ID
                          | LIT_ENTERO
                          | LIT_RACIONAL
                          | '(' <expresion_matematica> ')'
                          | <llamada_funcion>

```













### AUTÓMATA FINÍTO

<img width="991" height="1034" alt="4b17446d-03af-4933-8fac-8b1903dedc07" src="https://github.com/user-attachments/assets/9b5cbfa4-ad8d-4d8c-b6c7-5f46484bd735" />





