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

| Entrada | Salida esperada | Qué verifica |
|---|---|---|
