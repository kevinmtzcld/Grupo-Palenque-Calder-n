# Compilador 2026

## Integrantes
* **Calderón Martínez, Kevin**

## TP asignado

_GRUPO C_

## Lenguaje

- **Lenguaje fuente:** Razio (.raz), un lenguaje de programación imperativo y tipado que trabaja exclusivamente con números racionales (numerador/denominador).
- **Lenguaje de implementación:** C y Bison. El analizador léxico está desarrollado en C, el analizador sintáctico utiliza Bison (LALR(1)) y el generador de código está implementado en C.
- **Arquitectura destino:** Ensamblador NASM x86-64 para Windows (`win64`), enlazado con GCC a ejecutable nativo (`.exe`)[cite: 1].

## Cómo compilar

```bash
bison -d sintactico.y -o sintactico-en-c.c
gcc sintactico-en-c.c generador_asm.c -o compilador.exe
```

## Cómo ejecutar

```bash
./compilador.exe
./salida.exe
```

**Nota:** Todos los archivos fuente del compilador y el programa de entrada (`ejemplo.raz`) deben encontrarse en el mismo directorio desde donde se ejecutan los comandos. Al correr `./compilador.exe`, se generan automáticamente en esa misma carpeta `tabla_simbolos.txt`, `tercetos.txt`, `salida.asm`, `salida.obj` y `salida.exe`.

_(Asegurarse de que `ejemplo.raz` tenga el mismo nombre que el archivo indicado en la función `fopen()` del `main` del compilador)._

## Estructura del repositorio

- `docs/` — entregables formales de cada etapa
- `src/` — código del compilador
- `tests/` — casos de prueba
  - `correctos/` — programas que deben compilar sin errores
  - `errores-lexicos-sintacticos/`
  - `errores-semanticos/`
- `out/` — tercetos, tabla de símbolos exportada, `.asm` generado
- `specs/` — especificaciones y bitácoras por etapa
