``# Spec — Analizador léxico de UNO

**Grupo:** ejemplo de cátedra · **Lenguaje de implementación:** C
**Depende de:** `specs/01-diseno/spec.md`
**Produce:** `src/lexico/` · escribe en la tabla de símbolos definida en `specs/02-tabla-simbolos/spec.md`

---

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

f1 – Inicia identificador: limpia el buffer y guarda la primera letra leída.
f2 – Inicia constante racional: limpia el buffer y guarda el primer dígito leído.
f3a – Acumula carácter de identificador: Verifica que la longitud del buffer no supere el límite de identificadores (ej. 32 caracteres). Si está dentro del límite, concatena la letra o dígito; si lo excede, descarta los caracteres sobrantes.
f3b – Acumula carácter/dígito de constante: Acumula los dígitos o la barra / y verifica en tiempo real que el valor numérico (numerador o denominador) no supere el rango permitido D_2 (32 bits con signo). Activa la bandera de overflow si se excede.
f4 – Cierra identificador: verifica si el identificador es una palabra reservada. Si lo es, devuelve su token. Si no, busca el identificador en la tabla de símbolos y, si no existe, lo registra como ID.
f5 – Cierra constante racional: procesa el valor acumulado y verifica si la constante ya está almacenada. Si no existe, la registra en la tabla de constantes. Luego devuelve LIT_RACIONAL.
f7a – Inicia literal de cadena: Se ejecuta al leer la primera comilla doble ("). Limpia el buffer de cadenas para empezar de cero.
f7b – Acumula carácter de cadena: Verifica que la longitud no supere el límite máximo para cadenas. Si está dentro del límite, guarda cualquier carácter leído (incluyendo espacios); si lo excede, descarta o marca error de cadena demasiado larga.
f7c – Cierra literal de cadena: Se ejecuta al encontrar la comilla doble de cierre ("). Coloca el carácter nulo (\0) al final, registra la cadena en la Tabla de Cadenas/Literales y asigna el token LIT_CADENA.
f8 – Inicia el comentario y descarta todos sus caracteres hasta encontrar */: cuando se detecta el inicio de un comentario, se ignoran los caracteres que lo componen hasta encontrar la secuencia de cierre */. El comentario no genera ningún token.
Fn – Función nula: no realiza ninguna acción semántica ni genera ningún token. Se utiliza para ignorar espacios, tabulaciones y saltos de línea.

---

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
