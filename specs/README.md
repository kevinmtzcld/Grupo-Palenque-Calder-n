# Specs

Especificaciones y bitácoras de cada etapa del compilador.

## Quién escribió las specs

| Etapa | Autor(es) | Fecha |
|-------|-----------|-------|
| 01-diseno | Kevin Calderón | Septiembre 2026 |
| 02-tabla-simbolos | Kevin Calderón | Septiembre 2026 |
| 03-analizador-lexico | Kevin Calderón | Octubre 2026 |
| 04-analizador-sintactico | Kevin Calderón | Octubre 2026 |
| 05-errores | Kevin Calderón | Octubre 2026 |
| 06-codigo-intermedio | Kevin Calderón | Noviembre 2026 |
| 07-codigo-assembler | Kevin Calderón | Noviembre 2026 |

## Modelos usados

_(modelos de IA utilizados como asistencia, para qué se usaron y qué se revisó a mano)_

| Etapa | Modelo | Uso |
|-------|--------|-----|
| 01-diseno | Gemini / ChatGPT | Asistencia para estructurar las reglas léxicas, sintácticas y de tipos racionales de Razio. Se revisó a mano la compatibilidad con los requerimientos de la cátedra y la ausencia de ambigüedades. |
| 02-tabla-simbolos | Gemini | Apoyo en el diseño de la estructura de datos en C para almacenar identificadores, tipos y constantes racionales. Se validó manualmente el manejo de memoria, duplicados y exportación a `tabla_simbolos.txt`. |
| 03-analizador-lexico | Gemini | Consultas sobre expresiones regulares en Flex (detección de números racionales `a/b`, identificadores y comentarios) y diseño del AFD. Se probaron y ajustaron a mano todos los tokens y casos borde. |
| 04-analizador-sintactico | Gemini | Asistencia para resolver conflictos shift/reduce en Bison y estructurar la gramática libre de contexto (funciones, `si/sino`, `mientras`, `and`/`or`). Se verificó manualmente el árbol de derivación y las precedencias. |
| 05-errores | Gemini | Apoyo en la definición de validaciones semánticas (variables no declaradas, división por cero en racionales, alcance global) y recuperación de errores. Se testearon a mano todos los casos de error. |
| 06-codigo-intermedio | Gemini | Asistencia en el esquema de generación de tercetos, manejo de pilas para expresiones y saltos en cortocircuito (`and`/`or`). Se revisó y trazó a mano la numeración de tercetos y etiquetas. |
| 07-codigo-assembler | Gemini | Apoyo en la traducción de tercetos a ensamblador x86 (NASM) para aritmética de racionales (numerador/denominador, simplificación) y llamadas al sistema. Se depuró y probó manualmente `salida.asm` hasta generar `salida.exe`. |

## Convención

Cada carpeta de etapa contiene:

- `spec.md` — qué se va a construir y bajo qué reglas (contrato de la etapa)
- `bitacora.md` — registro cronológico de decisiones, problemas y cambios
