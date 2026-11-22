# Compilador 2026

## Integrantes

## Integrantes
* **Calderón Martínez, Kevin**

## TP asignado

_(número de trabajo práctico / variante asignada por la cátedra)_


## Lenguaje

- **Lenguaje fuente:** Razio (`.raz`), lenguaje imperativo tipado basado exclusivamente en números racionales (`numerador/denominador`).
- **Lenguaje de implementación:** C y Bison (analizador léxico manual en C, parser LALR(1) en Bison y generador de código intermedio/objeto en C)[cite: 1].
- **Arquitectura destino:** Ensamblador NASM x86-64 para Windows (`win64`), enlazado con GCC a ejecutable nativo (`.exe`)[cite: 1].


## Cómo compilar

```bash
bison -d sintactico.y -o sintactico-en-c.c
gcc sintactico-en-c.c generador_asm.c -o compilador.exe

## Cómo ejecutar

./compilador.exe
./salida.exe

Nota: Todos los archivos fuente del compilador y el programa de entrada (ejemplo.raz) deben encontrarse en el mismo directorio desde donde se ejecutan los comandos. Al correr ./compilador.exe, se generan automáticamente en esa misma carpeta tabla_simbolos.txt, tercetos.txt, salida.asm, salida.obj y salida.exe.


*(Asegurate de que el nombre `ejemplo.raz` coincida con el que tenés hardcodeado en el `fopen(...)` de tu `main`).*


## Estructura del repositorio

- `docs/` — entregables formales de cada etapa
- `src/` — código del compilador
- `tests/` — casos de prueba
  - `correctos/` — programas que deben compilar sin errores
  - `errores-lexicos-sintacticos/`
  - `errores-semanticos/`
- `out/` — tercetos, tabla de símbolos exportada, `.asm` generado
- `specs/` — especificaciones y bitácoras por etapa
