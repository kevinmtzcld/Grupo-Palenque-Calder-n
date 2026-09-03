## 1. Alfabeto

Σ = L ∪ D ∪ O ∪ S ∪ B

| Clase | Caracteres |
| :---: | :--- |
| **L** | `a – z`, `A – Z` |
| **D** | `0 – 9` |
| **O** | `+`, `-`, `*`, `/`, `=`, `<`, `>` |
| **S** | `(`, `)`, `{`, `}`, `;`, `:`, `,`, `"` |
| **B** | espacio, tabulación (`\t`), salto de línea (`\n`)|
| **OTRO** | cualquier otro carácter → error léxico |

**Cualquier carácter presente en el archivo .txt que no pertenezca a esta unión de conjuntos (por ejemplo: @, #, ~, ?, _, etc.) 
será rechazado por el analizador léxico, y generará un error léxico con el número de línea correspondiente**

## 2.PALABRAS RESERVADAS
def · principal  · racional · si · sino · mientras · mostrar · retornar

## 3. Tabla de tokens

|  Código | Token              | Lexema                          |
| :-----: | :----------------- | :------------------------------ |
| **257** | `PR_DEF`           | `def`                           |
| **258** | `PR_PRINCIPAL`     | `principal`                     |
| **259** | `PR_RACIONAL`      | `racional`                      |
| **260** | `PR_SI`            | `si`                            |
| **261** | `PR_SINO`          | `sino`                          |
| **262** | `PR_MIENTRAS`      | `mientras`                      |
| **263** | `PR_MOSTRAR`       | `mostrar`                       |
| **264** | `PR_RETORNAR`      | `retornar`                      |
| **265** | `ID`               | Identificador (`a`, `acum`)     |
| **266** | `LIT_RACIONAL`     | Constante racional (`3/4`, `5`) |
| **267** | `LIT_CADENA`       | Cadena de caracteres (`"Hola"`) |
| **268** | `OP_ASIG`          | `=`                             |
| **269** | `OP_SUMA`          | `+`                             |
| **270** | `OP_RESTA`         | `-`                             |
| **271** | `OP_MULT`          | `*`                             |
| **272** | `OP_DIV`           | `/`                             |
| **273** | `COMP_MAYOR`       | `>`                             |
| **274** | `COMP_MENOR`       | `<`                             |
| **275** | `COMP_MAYOR_IGUAL` | `>=`                            |
| **276** | `COMP_MENOR_IGUAL` | `<=`                            |
| **277** | `COMP_IGUAL`       | `==`                            |
| **278** | `COMP_DISTINTO`    | `!=`                            |
| **279** | `OP_AND`           | `and`                           |
| **280** | `OP_OR`            | `or`                            |
| **281** | `PAR_IZQ`          | `(`                             |
| **282** | `PAR_DER`          | `)`                             |
| **283** | `LLAVE_IZQ`        | `{`                             |
| **284** | `LLAVE_DER`        | `}`                             |
| **285** | `PUNTO_COMA`       | `;`                             |
| **286** | `DOS_PUNTOS`       | `:`                             |
| **287** | `COMA`             | `,`                             |
| **288** | `COMILLA`          | `"`                             |






## 4.PROGRAMA DE EJEMPLO
```
racional x,y,z=4/3,calculo,resultado,c=0;

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
```




## 5. GRAMÁTICA
```
<programa>::= <declaracion><funciones> PR_PRINCIPAL PAR_IZQ PAR_DER <bloque>

<declaracion> ::= PR_RACIONAL <lista_variables> PUNTO_COMA
<lista_variables> ::= <variable> | <variable> COMA <lista_variables>
<variable> ::= ID | ID OP_ASIG LIT_RACIONAL


<funciones> ::= lambda | <funcion> <funciones>
<funcion> ::= PR_DEF ID PAR_IZQ PAR_DER <bloque>
<bloque> ::= LLAVE_IZQ <sentencias> LLAVE_DER

<sentencias> ::= lambda | <sentencia> <sentencias>
<sentencia> ::= <asignacion> | <si>| <mientras>| <mostrar>| <retornar> | <llamada_funcion>

<asignacion> ::= ID OP_ASIG <expresion> PUNTO_COMA
<expresion> ::= <termino> | <expresion> OP_SUMA <termino> | <expresion> OP_RESTA <termino>
<termino> ::= <factor> | <termino> OP_MULT <factor> | <termino> OP_DIV <factor>
<factor> ::= LIT_RACIONAL | ID | PAR_IZQ <expresion> PAR_DER | ID PAR_IZQ PAR_DER

<si> ::= PR_SI PAR_IZQ <condicion> PAR_DER <bloque> PR_SINO <bloque>
<mientras> ::= PR_MIENTRAS PAR_IZQ <condicion> PAR_DER <bloque>

<mostrar> ::= PR_MOSTRAR PAR_IZQ <argumentos_mostrar> PAR_DER PUNTO_COMA
<argumentos_mostrar> ::= ID | COMILLA LIT_CADENA COMILLA | COMILLA LIT_CADENA COMILLA COMA <lista_ids_mostrar>
<lista_ids_mostrar> ::= ID | ID COMA <lista_ids_mostrar>

<retornar> ::= PR_RETORNAR PAR_IZQ <valor_retorno> PAR_DER PUNTO_COMA
<valor_retorno> ::= ID | LIT_RACIONAL

<llamada_funcion> ::= ID PAR_IZQ PAR_DER PUNTO_COMA



<condicion> ::= <cond_or>

<cond_or> ::= <cond_and> | <cond_or> OP_OR <cond_and>

<cond_and> ::= <cond_factor> | <cond_and> OP_AND <cond_factor>

<cond_factor> ::= PAR_IZQ <condicion> PAR_DER | <comparacion>

<comparacion> ::= <expresion> <operador_comparacion> <expresion>

<operador_comparacion> ::= COMP_MAYOR | COMP_MENOR | COMP_MAYOR_IGUAL | COMP_MENOR_IGUAL | COMP_IGUAL | COMP_DISTINTO


```













### AUTÓMATA FINÍTO

<img width="991" height="1034" alt="4b17446d-03af-4933-8fac-8b1903dedc07" src="https://github.com/user-attachments/assets/9b5cbfa4-ad8d-4d8c-b6c7-5f46484bd735" />





