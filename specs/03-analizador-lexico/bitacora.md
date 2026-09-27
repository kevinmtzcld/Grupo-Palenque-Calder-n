# Bitácora de Desarrollo — Analizador Léxico (Razio)

**Grupo:** Grupo C  
**Spec de referencia:** `specs/01-diseno/spec.md`  
**Modelos usados:** Gemini  

---

## Iteración 0 — Primer borrador del léxico

- **Fecha:** 20/09/2026 · Gemini
- **Qué pedí:** Le pasé los archivos de la materia (`paralexico.txt`, la tabla de tokens, el alfabeto y las reglas semánticas f1 a f8) y las matrices (de estados, de tokens, y de funciones) y le pedí que me arme la primera versión del analizador léxico en C.
- **Qué devolvió:** Un código repartido en varios archivos que leía el código de prueba y reconocía tokens, pero que adentro usaba un `switch(estado)` para saber qué hacer en cada momento.
- **Qué cambié y por qué:** Lo probé en OnlineGDB con un archivo de prueba `ejemplo.raz`. Aunque andaba, cuando lo revisé con el profesor me marcó dos cosas clave:
  1. No podía usar `switch` o `if` para ejecutar las acciones; tenía que usar sí o sí la matriz de punteros a función que exige la materia
  2. Separar todo en archivos `.h` antes de tiempo era sobrecomplicar la entrega.
- **Impacto en la spec:** Dado que el codigo si cumplia con el objetivo final (Generar los tokens correctamente del programa ejemplo) pero no hacia uso de la matriz de funciones, y el codigo me lo dió en archivos lexico.h y tokens.h ,tendré que pedir que lo rehaga con los cambios marcados

---

## Iteración 1 — Cambio de arquitectura a matriz de funciones

- **Fecha:** 27/09/2026 · Gemini
- **Qué pedí:** Rehacer el código para corregir las indicaciones: meter las funciones en la matriz de punteros y pasar todo a un solo archivo `main.c`.
- **Qué devolvió:** El código en un solo `main.c` donde las acciones semánticas ($f1-f8$) se ejecutan directamente llamando a la celda de la matriz
- **Qué cambié y por qué:** Acepté esta versión porque era exactamente la forma teórica que pedía la cátedra
- **Impacto en la spec:** Acomodé la documentación técnica del proyecto colocando el archivo `spec.md` en la carpeta `specs/01-diseno/`[cite: 3].

---

## Iteración 2 — Arreglo del texto guardado en los símbolos simples

- **Fecha:** 27/09/2026 · Gemini
- **Qué pedí:** Le mostré la salida de la pantalla porque noté un error: cuando leía un símbolo simple (como un `=`, un `;` o un paréntesis), imprimía el texto del token que había leído antes en lugar del símbolo actual.
- **Qué devolvió:** Encontró que la función nula `fn()` no estaba guardando nada en el buffer cuando leía estos símbolos rápidos.
- **Qué cambié y por qué:** Modificamos la función `fn()` para que guarde el símbolo si el buffer está vacío y aseguramos reiniciar el índice del buffer al comienzo de cada token. Con esto cada símbolo salió impreso correctamente.
- **Impacto en la spec:** Ninguno.

---
