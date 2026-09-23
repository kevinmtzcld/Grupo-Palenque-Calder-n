### Spec — Analizador léxico de UNO
**Grupo C**  · **Lenguaje de implementación:** C
**Depende de:** `specs/01-diseno/spec.md`
**Produce:** `src/lexico/` · escribe en la tabla de símbolos definida en `specs/02-tabla-simbolos/spec.md`

## 1. Alcance e interfaz

Función `yylex()` invocada por el analizador sintáctico. Devuelve **un token por
llamada**, como entero asociado al número de token. No es una pasada previa que
produzca la lista completa.

## 2. Decisiones propias de esta fase

| # | Decisión | Valor |
|---|---|---|

---

## 3. Eventos (columnas de las matrices)

`get_evento(c)` mapea el carácter leído a una columna:

| Col | Evento | Caracteres |
|---|---|---|

---

## 4. Estados y Diagrama

| Estado | Significado |
|---|---|

---

## 5. Acciones semánticas

| Acción | Qué hace |
|---|---|
| `f1` | **Inicia identificador:** Limpia el buffer de identificadores y guarda la primera letra leída. |
| `f2` | **Inicia constante racional:** Limpia el buffer de números y guarda el primer dígito leído. |
| `f3a` | **Acumula carácter de identificador:** Verifica que la longitud del buffer no supere el límite permitido para identificadores (ej. 32 caracteres). Si está dentro del límite, concatena la letra o dígito; si lo excede, descarta los caracteres sobrantes. |
| `f3b` | **Acumula carácter/dígito de constante:** Acumula dígitos o el carácter `/` y verifica en tiempo real que el valor numérico (numerador o denominador) no supere el rango permitido de 32 bits con signo ($D_2$). Activa la bandera de overflow si se excede. |
| `f4` | **Cierra identificador:** Verifica si el identificador es una palabra reservada. Si lo es, devuelve su token. Si no, busca el identificador en la Tabla de Símbolos y, si no existe, lo registra como `ID`. |
| `f5` | **Cierra constante racional:** Procesa el valor acumulado en el buffer, simplifica la fracción por MCD, verifica rango $D_2$ y denominador $\neq 0$. Si no existe en la Tabla de Constantes, la registra y devuelve `LIT_RACIONAL`. |
| `f7a` | **Inicia literal de cadena:** Se ejecuta al leer la primera comilla doble (`"`). Limpia el buffer de cadenas para empezar de cero. |
| `f7b` | **Acumula carácter de cadena:** Verifica que la longitud no supere el límite máximo para cadenas. Si está dentro del límite, guarda cualquier carácter leído (incluyendo espacios); si lo excede, descarta o marca error. |
| `f7c` | **Cierra literal de cadena:** Se ejecuta al encontrar la comilla doble de cierre (`"`). Coloca el carácter nulo (`\0`) al final, remueve comillas exteriores, registra la cadena en la Tabla de Cadenas/Literales y asigna el token `LIT_CADENA`. |
| `f8` | **Ignora caracteres de comentario:** Se ejecuta al transitar por el cuerpo de un comentario multilínea (`/* ... */`). Descarta los caracteres leídos sin guardarlos en buffer ni generar ningún token, permitiendo avanzar entre estados hasta encontrar `*/`. |
| `fn` | **Función nula:** No realiza ninguna acción semántica ni genera ningún token. Se utiliza para avanzar y descartar espacios en blanco, tabulaciones y saltos de línea, o en transiciones puras de control. |
| `fe` | **Función de error:** Reporta un error léxico cuando se encuentra un carácter inválido o una secuencia no reconocida, registrando la línea correspondiente. |




## 6. Matriz de Nuevos Estados
## 7. Matriz de Transiciones
## 8. Tabla de Unreads
[Matrices.xlsx](https://github.com/user-attachments/files/32564330/Matrices.xlsx)
---

## 9. Errores que emite esta fase

| Código | Condición | Mensaje |
|---|---|---|

---

## 10. Traza de verificación

Entrada ``, con el estado inicial 0:

| Estado | Lee | Evento | Acción | Nuevo estado | Unread | Retorna |
|---|---|---|---|---|---|---|


---

## 11. Casos de prueba de esta fase

### Programa de ejemplo (`ejemplo.raz`)

```c
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

=================================================================
     INICIANDO ANALIZADOR LÉXICO - COMPILADOR RAZIO (GRUPO C)    
=================================================================

[Línea  1] TOKEN: PR_RACIONAL        | Código: 259
[Línea  1] TOKEN: ID                 | Código: 265 | Lexema: "x"
[Línea  1] TOKEN: COMA               | Código: 286
[Línea  1] TOKEN: ID                 | Código: 265 | Lexema: "y"
[Línea  1] TOKEN: COMA               | Código: 286
[Línea  1] TOKEN: ID                 | Código: 265 | Lexema: "z"
[Línea  1] TOKEN: OP_ASIG            | Código: 268
[Línea  1] TOKEN: LIT_RACIONAL       | Código: 266 | Lexema: "4/3" (num: 4, den: 3)
[Línea  1] TOKEN: COMA               | Código: 286
[Línea  1] TOKEN: ID                 | Código: 265 | Lexema: "calculo"
[Línea  1] TOKEN: COMA               | Código: 286
[Línea  1] TOKEN: ID                 | Código: 265 | Lexema: "resultado"
[Línea  1] TOKEN: COMA               | Código: 286
[Línea  1] TOKEN: ID                 | Código: 265 | Lexema: "c"
[Línea  1] TOKEN: OP_ASIG            | Código: 268
[Línea  1] TOKEN: LIT_RACIONAL       | Código: 266 | Lexema: "0" (num: 0, den: 1)
[Línea  1] TOKEN: PUNTO_COMA         | Código: 285
[Línea  5] TOKEN: PR_DEF             | Código: 257
[Línea  5] TOKEN: ID                 | Código: 265 | Lexema: "funX"
[Línea  5] TOKEN: PAR_IZQ            | Código: 281
[Línea  5] TOKEN: PAR_DER            | Código: 282
[Línea  6] TOKEN: LLAVE_IZQ          | Código: 283
[Línea  7] TOKEN: PR_MOSTRAR         | Código: 263
[Línea  7] TOKEN: PAR_IZQ            | Código: 281
[Línea  7] TOKEN: LIT_CADENA         | Código: 267 | Lexema: "Calculando"
[Línea  7] TOKEN: PAR_DER            | Código: 282
[Línea  7] TOKEN: PUNTO_COMA         | Código: 285
[Línea  8] TOKEN: ID                 | Código: 265 | Lexema: "calculo"
[Línea  8] TOKEN: OP_ASIG            | Código: 268
[Línea  8] TOKEN: ID                 | Código: 265 | Lexema: "x"
[Línea  8] TOKEN: OP_SUMA            | Código: 269
[Línea  8] TOKEN: ID                 | Código: 265 | Lexema: "y"
[Línea  8] TOKEN: PUNTO_COMA         | Código: 285
[Línea  9] TOKEN: PR_RETORNAR        | Código: 264
[Línea  9] TOKEN: PAR_IZQ            | Código: 281
[Línea  9] TOKEN: ID                 | Código: 265 | Lexema: "calculo"
[Línea  9] TOKEN: PAR_DER            | Código: 282
[Línea  9] TOKEN: PUNTO_COMA         | Código: 285
[Línea 10] TOKEN: LLAVE_DER          | Código: 284
[Línea 12] TOKEN: PR_PRINCIPAL       | Código: 258
[Línea 12] TOKEN: PAR_IZQ            | Código: 281
[Línea 12] TOKEN: PAR_DER            | Código: 282
[Línea 13] TOKEN: LLAVE_IZQ          | Código: 283
[Línea 14] TOKEN: ID                 | Código: 265 | Lexema: "x"
[Línea 14] TOKEN: OP_ASIG            | Código: 268
[Línea 14] TOKEN: LIT_RACIONAL       | Código: 266 | Lexema: "9/2" (num: 9, den: 2)
[Línea 14] TOKEN: PUNTO_COMA         | Código: 285
[Línea 15] TOKEN: ID                 | Código: 265 | Lexema: "y"
[Línea 15] TOKEN: OP_ASIG            | Código: 268
[Línea 15] TOKEN: LIT_RACIONAL       | Código: 266 | Lexema: "9/8" (num: 9, den: 8)
[Línea 15] TOKEN: PUNTO_COMA         | Código: 285
[Línea 17] TOKEN: ID                 | Código: 265 | Lexema: "resultado"
[Línea 17] TOKEN: OP_ASIG            | Código: 268
[Línea 17] TOKEN: ID                 | Código: 265 | Lexema: "funX"
[Línea 17] TOKEN: PAR_IZQ            | Código: 281
[Línea 17] TOKEN: PAR_DER            | Código: 282
[Línea 17] TOKEN: PUNTO_COMA         | Código: 285
[Línea 19] TOKEN: PR_SI              | Código: 260
[Línea 19] TOKEN: PAR_IZQ            | Código: 281
[Línea 19] TOKEN: PAR_IZQ            | Código: 281
[Línea 19] TOKEN: ID                 | Código: 265 | Lexema: "x"
[Línea 19] TOKEN: COMP_MAYOR         | Código: 273
[Línea 19] TOKEN: ID                 | Código: 265 | Lexema: "y"
[Línea 19] TOKEN: PAR_DER            | Código: 282
[Línea 19] TOKEN: OP_AND             | Código: 279
[Línea 19] TOKEN: PAR_IZQ            | Código: 281
[Línea 19] TOKEN: ID                 | Código: 265 | Lexema: "c"
[Línea 19] TOKEN: COMP_MENOR         | Código: 274
[Línea 19] TOKEN: LIT_RACIONAL       | Código: 266 | Lexema: "3" (num: 3, den: 1)
[Línea 19] TOKEN: PAR_DER            | Código: 282
[Línea 19] TOKEN: PAR_DER            | Código: 282
[Línea 20] TOKEN: LLAVE_IZQ          | Código: 283
[Línea 21] TOKEN: PR_MOSTRAR         | Código: 263
[Línea 21] TOKEN: PAR_IZQ            | Código: 281
[Línea 21] TOKEN: LIT_CADENA         | Código: 267 | Lexema: "x es mayor a y"
[Línea 21] TOKEN: PAR_DER            | Código: 282
[Línea 21] TOKEN: PUNTO_COMA         | Código: 285
[Línea 23] TOKEN: PR_MIENTRAS        | Código: 262
[Línea 23] TOKEN: PAR_IZQ            | Código: 281
[Línea 23] TOKEN: PAR_IZQ            | Código: 281
[Línea 23] TOKEN: ID                 | Código: 265 | Lexema: "c"
[Línea 23] TOKEN: COMP_MENOR         | Código: 274
[Línea 23] TOKEN: LIT_RACIONAL       | Código: 266 | Lexema: "3" (num: 3, den: 1)
[Línea 23] TOKEN: PAR_DER            | Código: 282
[Línea 23] TOKEN: OP_OR              | Código: 280
[Línea 23] TOKEN: PAR_IZQ            | Código: 281
[Línea 23] TOKEN: ID                 | Código: 265 | Lexema: "x"
[Línea 23] TOKEN: COMP_MENOR         | Código: 274
[Línea 23] TOKEN: ID                 | Código: 265 | Lexema: "y"
[Línea 23] TOKEN: PAR_DER            | Código: 282
[Línea 23] TOKEN: PAR_DER            | Código: 282
[Línea 24] TOKEN: LLAVE_IZQ          | Código: 283
[Línea 25] TOKEN: PR_MOSTRAR         | Código: 263
[Línea 25] TOKEN: PAR_IZQ            | Código: 281
[Línea 25] TOKEN: LIT_CADENA         | Código: 267 | Lexema: "valor de c:"
[Línea 25] TOKEN: COMA               | Código: 286
[Línea 25] TOKEN: ID                 | Código: 265 | Lexema: "c"
[Línea 25] TOKEN: PAR_DER            | Código: 282
[Línea 25] TOKEN: PUNTO_COMA         | Código: 285
[Línea 26] TOKEN: ID                 | Código: 265 | Lexema: "c"
[Línea 26] TOKEN: OP_ASIG            | Código: 268
[Línea 26] TOKEN: ID                 | Código: 265 | Lexema: "c"
[Línea 26] TOKEN: OP_SUMA            | Código: 269
[Línea 26] TOKEN: LIT_RACIONAL       | Código: 266 | Lexema: "1" (num: 1, den: 1)
[Línea 26] TOKEN: PUNTO_COMA         | Código: 285
[Línea 27] TOKEN: LLAVE_DER          | Código: 284
[Línea 28] TOKEN: LLAVE_DER          | Código: 284
[Línea 29] TOKEN: PR_SINO            | Código: 261
[Línea 30] TOKEN: LLAVE_IZQ          | Código: 283
[Línea 31] TOKEN: PR_SI              | Código: 260
[Línea 31] TOKEN: PAR_IZQ            | Código: 281
[Línea 31] TOKEN: ID                 | Código: 265 | Lexema: "x"
[Línea 31] TOKEN: COMP_MENOR         | Código: 274
[Línea 31] TOKEN: ID                 | Código: 265 | Lexema: "y"
[Línea 31] TOKEN: PAR_DER            | Código: 282
[Línea 32] TOKEN: LLAVE_IZQ          | Código: 283
[Línea 33] TOKEN: PR_MOSTRAR         | Código: 263
[Línea 33] TOKEN: PAR_IZQ            | Código: 281
[Línea 33] TOKEN: LIT_CADENA         | Código: 267 | Lexema: "Y es mayor a x"
[Línea 33] TOKEN: PAR_DER            | Código: 282
[Línea 33] TOKEN: PUNTO_COMA         | Código: 285
[Línea 34] TOKEN: LLAVE_DER          | Código: 284
[Línea 35] TOKEN: PR_SINO            | Código: 261
[Línea 36] TOKEN: LLAVE_IZQ          | Código: 283
[Línea 37] TOKEN: PR_MOSTRAR         | Código: 263
[Línea 37] TOKEN: PAR_IZQ            | Código: 281
[Línea 37] TOKEN: LIT_CADENA         | Código: 267 | Lexema: "X e y son iguales"
[Línea 37] TOKEN: PAR_DER            | Código: 282
[Línea 37] TOKEN: PUNTO_COMA         | Código: 285
[Línea 38] TOKEN: LLAVE_DER          | Código: 284
[Línea 39] TOKEN: LLAVE_DER          | Código: 284
[Línea 40] TOKEN: LLAVE_DER          | Código: 284

=================================================================
                PROCESAMIENTO LÉXICO FINALIZADO                  
=================================================================


| Entrada | Salida esperada | Qué verifica |
|---|---|---|
