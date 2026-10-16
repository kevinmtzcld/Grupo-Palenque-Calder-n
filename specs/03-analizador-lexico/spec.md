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


### 7. Acciones semánticas (funciones `f1` a `fe`)

Implementan la lógica asociada a las transiciones del autómata léxico.

| Función | Descripción |
|---|---|
| `f1` | Inicializa el buffer para el reconocimiento de identificadores (`ID`). |
| `f2` | Inicializa el buffer para el reconocimiento de constantes racionales. |
| `f3a` | Acumula caracteres en identificadores, aplicando la regla D6: truncamiento a un máximo de 20 caracteres. |
| `f3b` | Acumula dígitos y la barra `/` en constantes racionales, verificando el rango de 32 bits según la regla D2. |
| `f4` | Reconoce palabras reservadas o inserta identificadores en la Tabla de Símbolos. |
| `f5` | Finaliza el reconocimiento de una constante racional, valida la división por cero durante la compilación e inserta la constante en la Tabla de Símbolos. |
| `f7a`, `f7b`, `f7c` | Gestionan la captura y acumulación de literales de cadena delimitados por comillas dobles (`"..."`). |
| `f8` | Descarta comentarios multilínea y limpia el buffer correspondiente. |
| `fa` | Acumula explícitamente el segundo carácter de los operadores compuestos, como `==`, `!=`, `>=` y `<=`. |
| `fn` | Función nula utilizada en separadores y espacios en blanco, sin realizar acciones semánticas. |
| `fu` | Finaliza el reconocimiento de un token mediante retracción (*retract*), devolviendo el carácter leído de más al flujo de entrada para que pueda ser procesado en la siguiente transición. |
| `fe` | Captura y reporta errores léxicos. |





## 6. Matriz de Nuevos Estados
| DESCRIPCIÓN | ESTADO | Letra | Dígito | + | - | * | / | = | < | > | ! | ( | ) | { | } | ; | : | , | " | Blanco | Otro |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| inicio | e0 | e1 | e4 | e25 | e26 | e27 | e15 | e11 | e9 | e7 | e2 | e21 | e22 | e23 | e24 | e18 | error | e19 | e13 | e0 | error |
| id o palabra | e1 | e1 | e1 | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | error |
| posible distinto | e2 | error | error | error | error | error | error | e3 | error | error | error | error | error | error | error | error | error | error | error | error | error |
| op distinto confirmado | e3 | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | error |
| num racional | e4 | f | e4 | f | f | f | e5 | f | f | f | f | f | f | f | f | f | f | f | f | f | error |
| denominador | e5 | error | e6 | error | error | error | error | error | error | error | error | error | error | error | error | error | error | error | error | error | error |
| acum denominador | e6 | f | e6 | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | error |
| mayor o mayor igual | e7 | f | f | f | f | f | f | e8 | f | f | f | f | f | f | f | f | f | f | f | f | error |
| mayor igual | e8 | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | error |
| menor o menor igual | e9 | f | f | f | f | f | f | e10 | f | f | f | f | f | f | f | f | f | f | f | f | error |
| menor igual | e10 | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | error |
| posible asignación | e11 | f | f | f | f | f | f | e12 | f | f | f | f | f | f | f | f | f | f | f | f | error |
| comp igualdad | e12 | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | error |
| inicio cadena literal | e13 | e13 | e13 | e13 | e13 | e13 | e13 | e13 | e13 | e13 | e13 | e13 | e13 | e13 | e13 | e13 | e13 | e13 | e14 | e13 | error |
| fin cadena literal | e14 | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | error |
| división o comentario | e15 | f | f | f | f | e16 | f | f | f | f | f | f | f | f | f | f | f | f | f | f | error |
| comentario | e16 | e16 | e16 | e16 | e16 | e17 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | error |
| posible fin de comentario | e17 | e16 | e16 | e16 | e16 | e16 | e0 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | error |
| punto coma | e18 | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | error |
| coma | e19 | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | error |
| parentesis abre | e21 | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | error |
| parentesis cierra | e22 | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | error |
| llave abre | e23 | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | error |
| llave cierra | e24 | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | error |
| suma | e25 | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | error |
| resta | e26 | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | error |
| multiplicación | e27 | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | f | error |




## 8.Matriz de funciones

| DESCRIPCIÓN | ESTADO | Letra | Dígito | + | - | * | / | = | < | > | ! | ( | ) | { | } | ; | : | , | " | Blanco | Otro |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| inicio | e0 | e1-f1 | e4-f2 | e25-fn | e26-fn | e27-fn | e15-fn | e11-fn | e9-fn | e7-fn | e2-fn | e21-fn | e22-fn | e23-fn | e24-fn | e18-fn | error | e19-fn | e13-f7a | e0-fn | error-fe |
| id o palabra | e1 | e1-f3a | e1-f3a | fu-f4 | fu-f4 | fu-f4 | fu-f4 | fu-f4 | fu-f4 | fu-f4 | fu-f4 | fu-f4 | fu-f4 | fu-f4 | fu-f4 | fu-f4 | fu-f4 | fu-f4 | fu-f4 | fu-f4 | error-fe |
| posible distinto | e2 | error-fe | error-fe | error-fe | error-fe | error-fe | error-fe | e3-fa | error-fe | error-fe | error-fe | error-fe | error-fe | error-fe | error-fe | error-fe | error-fe | error-fe | error-fe | error-fe | error-fe |
| op distinto confirmado | e3 | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | error-fe |
| num racional | e4 | fu-f5 | e4-f3b | fu-f5 | fu-f5 | fu-f5 | e5-f3b | fu-f5 | fu-f5 | fu-f5 | fu-f5 | fu-f5 | fu-f5 | fu-f5 | fu-f5 | fu-f5 | fu-f5 | fu-f5 | fu-f5 | fu-f5 | error-fe |
| denominador | e5 | error-fe | e6-f3b | error-fe | error-fe | error-fe | error-fe | error-fe | error-fe | error-fe | error-fe | error-fe | error-fe | error-fe | error-fe | error-fe | error-fe | error-fe | error-fe | error-fe | error-fe |
| acum denominador | e6 | fu-f5 | e6-f3b | fu-f5 | fu-f5 | fu-f5 | fu-f5 | fu-f5 | fu-f5 | fu-f5 | fu-f5 | fu-f5 | fu-f5 | fu-f5 | fu-f5 | fu-f5 | fu-f5 | fu-f5 | fu-f5 | fu-f5 | error-fe |
| mayor o mayor igual | e7 | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | e8-fa | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | error-fe |
| mayor igual | e8 | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | error-fe |
| menor o menor igual | e9 | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | e10-fa | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | error-fe |
| menor igual | e10 | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | error-fe |
| posible asignación | e11 | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | e12-fa | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | error-fe |
| comp igualdad | e12 | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | error-fe |
| inicio cadena literal | e13 | e13-f7b | e13-f7b | e13-f7b | e13-f7b | e13-f7b | e13-f7b | e13-f7b | e13-f7b | e13-f7b | e13-f7b | e13-f7b | e13-f7b | e13-f7b | e13-f7b | e13-f7b | e13-f7b | e13-f7b | e14-f7c | e13-f7b | error-fe |
| fin cadena literal | e14 | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | error-fe |
| división o comentario | e15 | fu-fn | fu-fn | fu-fn | fu-fn | e16-f8 | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | error-fe |
| comentario | e16 | e16-f8 | e16-f8 | e16-f8 | e16-f8 | e17-f8 | e16-f8 | e16-f8 | e16-f8 | e16-f8 | e16-f8 | e16-f8 | e16-f8 | e16-f8 | e16-f8 | e16-f8 | e16-f8 | e16-f8 | e16-f8 | e16-f8 | error-fe |
| posible fin de comentario | e17 | e16-f8 | e16-f8 | e16-f8 | e16-f8 | e16-f8 | e0-f8 | e16-f8 | e16-f8 | e16-f8 | e16-f8 | e16-f8 | e16-f8 | e16-f8 | e16-f8 | e16-f8 | e16-f8 | e16-f8 | e16-f8 | e16-f8 | error-fe |
| punto coma | e18 | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | error-fe |
| coma | e19 | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | error-fe |
| parentesis abre | e21 | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | error-fe |
| parentesis cierra | e22 | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | error-fe |
| llave abre | e23 | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | error-fe |
| llave cierra | e24 | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | error-fe |
| suma | e25 | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | error-fe |
| resta | e26 | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | error-fe |
| multiplicación | e27 | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | fu-fn | error-fe |
---



## Matriz de tokens

| DESCRIPCIÓN | ESTADO | Letra | Dígito | + | - | * | / | = | < | > | ! | ( | ) | { | } | ; | : | , | " | Blanco | Otro |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| inicio | e0 | e1 | e4 | e25 | e26 | e27 | e15 | e11 | e9 | e7 | e2 | e21 | e22 | e23 | e24 | e18 | error | e19 | e13 | e0 | error |
| id o palabra | e1 | e1 | e1 | 265 | 265 | 265 | 265 | 265 | 265 | 265 | 265 | 265 | 265 | 265 | 265 | 265 | 265 | 265 | 265 | 265 | error |
| posible distinto | e2 | error | error | error | error | error | error | e3 | error | error | error | error | error | error | error | error | error | error | error | error | error |
| op distinto confirmado | e3 | 278 | 278 | 278 | 278 | 278 | 278 | 278 | 278 | 278 | 278 | 278 | 278 | 278 | 278 | 278 | 278 | 278 | 278 | 278 | error |
| num racional | e4 | 266 | e4 | 266 | 266 | 266 | e5 | 266 | 266 | 266 | 266 | 266 | 266 | 266 | 266 | 266 | 266 | 266 | 266 | 266 | error |
| denominador | e5 | error | e6 | error | error | error | error | error | error | error | error | error | error | error | error | error | error | error | error | error | error |
| acum denominador | e6 | 266 | e6 | 266 | 266 | 266 | 266 | 266 | 266 | 266 | 266 | 266 | 266 | 266 | 266 | 266 | 266 | 266 | 266 | 266 | error |
| mayor o mayor igual | e7 | 273 | 273 | 273 | 273 | 273 | 273 | e8 | 273 | 273 | 273 | 273 | 273 | 273 | 273 | 273 | 273 | 273 | 273 | 273 | error |
| mayor igual | e8 | 275 | 275 | 275 | 275 | 275 | 275 | 275 | 275 | 275 | 275 | 275 | 275 | 275 | 275 | 275 | 275 | 275 | 275 | 275 | error |
| menor o menor igual | e9 | 274 | 274 | 274 | 274 | 274 | 274 | e10 | 274 | 274 | 274 | 274 | 274 | 274 | 274 | 274 | 274 | 274 | 274 | 274 | error |
| menor igual | e10 | 276 | 276 | 276 | 276 | 276 | 276 | 276 | 276 | 276 | 276 | 276 | 276 | 276 | 276 | 276 | 276 | 276 | 276 | 276 | error |
| posible asignación | e11 | 268 | 268 | 268 | 268 | 268 | 268 | e12 | 268 | 268 | 268 | 268 | 268 | 268 | 268 | 268 | 268 | 268 | 268 | 268 | error |
| comp igualdad | e12 | 277 | 277 | 277 | 277 | 277 | 277 | 277 | 277 | 277 | 277 | 277 | 277 | 277 | 277 | 277 | 277 | 277 | 277 | 277 | error |
| inicio cadena literal | e13 | e13 | e13 | e13 | e13 | e13 | e13 | e13 | e13 | e13 | e13 | e13 | e13 | e13 | e13 | e13 | e13 | e13 | e14 | e13 | error |
| fin cadena literal | e14 | 267 | 267 | 267 | 267 | 267 | 267 | 267 | 267 | 267 | 267 | 267 | 267 | 267 | 267 | 267 | 267 | 267 | 267 | 267 | error |
| división o comentario | e15 | 272 | 272 | 272 | 272 | e16 | 272 | 272 | 272 | 272 | 272 | 272 | 272 | 272 | 272 | 272 | 272 | 272 | 272 | 272 | error |
| comentario | e16 | e16 | e16 | e16 | e16 | e17 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | error |
| posible fin de comentario | e17 | e16 | e16 | e16 | e16 | e16 | e0 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | e16 | error |
| punto coma | e18 | 285 | 285 | 285 | 285 | 285 | 285 | 285 | 285 | 285 | 285 | 285 | 285 | 285 | 285 | 285 | 285 | 285 | 285 | 285 | error |
| coma | e19 | 286 | 286 | 286 | 286 | 286 | 286 | 286 | 286 | 286 | 286 | 286 | 286 | 286 | 286 | 286 | 286 | 286 | 286 | 286 | error |
| parentesis abre | e21 | 281 | 281 | 281 | 281 | 281 | 281 | 281 | 281 | 281 | 281 | 281 | 281 | 281 | 281 | 281 | 281 | 281 | 281 | 281 | error |
| parentesis cierra | e22 | 282 | 282 | 282 | 282 | 282 | 282 | 282 | 282 | 282 | 282 | 282 | 282 | 282 | 282 | 282 | 282 | 282 | 282 | 282 | error |
| llave abre | e23 | 283 | 283 | 283 | 283 | 283 | 283 | 283 | 283 | 283 | 283 | 283 | 283 | 283 | 283 | 283 | 283 | 283 | 283 | 283 | error |
| llave cierra | e24 | 284 | 284 | 284 | 284 | 284 | 284 | 284 | 284 | 284 | 284 | 284 | 284 | 284 | 284 | 284 | 284 | 284 | 284 | 284 | error |
| suma | e25 | 269 | 269 | 269 | 269 | 269 | 269 | 269 | 269 | 269 | 269 | 269 | 269 | 269 | 269 | 269 | 269 | 269 | 269 | 269 | error |
| resta | e26 | 270 | 270 | 270 | 270 | 270 | 270 | 270 | 270 | 270 | 270 | 270 | 270 | 270 | 270 | 270 | 270 | 270 | 270 | 270 | error |
| multiplicación | e27 | 271 | 271 | 271 | 271 | 271 | 271 | 271 | 271 | 271 | 271 | 271 | 271 | 271 | 271 | 271 | 271 | 271 | 271 | 271 | error |





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
```

```

