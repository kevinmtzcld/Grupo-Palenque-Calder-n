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

_(Modelos de IA utilizados, para qué se usaron y qué se verificó manualmente)_

| Etapa | Modelo | Uso |
|-------|--------|-----|
| 01-diseno | Gemini / ChatGPT | Ayuda para redactar y organizar la especificación del lenguaje. Se revisó que las reglas fueran correctas y cumplieran con las consignas. |
| 02-tabla-simbolos | Gemini | Ayuda para diseñar la estructura de datos y el formato de exportación. Se verificó el manejo de memoria y se probaron distintos casos. |
| 03-analizador-lexico | Gemini | Consultas sobre el reconocimiento de tokens y los casos especiales. Se revisaron y probaron las reglas léxicas. |
| 04-analizador-sintactico | Gemini | Ayuda para revisar la gramática en Bison y resolver conflictos. Se comprobaron la precedencia de operadores y las estructuras de control. |
| 05-errores | Gemini | Ayuda para organizar las validaciones de errores léxicos, sintácticos y semánticos. Se probaron los distintos casos y mensajes de error. |
| 06-codigo-intermedio | Gemini | Consultas sobre la generación de tercetos y los saltos condicionales. Se revisó manualmente la salida de `tercetos.txt`. |
| 07-codigo-assembler | Gemini | Ayuda para traducir los tercetos a assembler y corregir errores de compilación. Se verificó la ejecución del programa generado. |

## Convención

Cada carpeta de etapa contiene:

- `spec.md` — qué se va a construir y bajo qué reglas (contrato de la etapa)
- `bitacora.md` — registro cronológico de decisiones, problemas y cambios
