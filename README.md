# Compilador 2026

## Integrantes
* **Calderón Martínez, Kevin**

## TP asignado

_GRUPO C_

## Lenguaje

- **Lenguaje fuente:** Razio (.raz), un lenguaje de programación imperativo y tipado que trabaja exclusivamente con números racionales (numerador/denominador).
- **Lenguaje de implementación:** C y Bison. El analizador léxico está desarrollado en C, el analizador sintáctico utiliza Bison (LALR(1)) y el generador de código está implementado en C.
- **Arquitectura destino:** Ensamblador NASM x86-64 para Windows (`win64`), enlazado con GCC a ejecutable nativo (`.exe`)

## Cómo compilar
Ingresar a la carpeta `src/` y ejecutar:

```bash
bison -d sintactico.y
gcc -o compilador.exe sintactico.tab.c alexico.c generador_asm.c
```

## Cómo ejecutar
Ubicar el archivo de entrada deseado(tomado de la carpeta tests/ y renombrado a ejemplo.raz) en el mismo directorio que compilador.exe y ejecutar:

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
  - `correctos` — programas válidos que compilan sin errores
  - `ejemplo.raz` — programa base utilizado para generar los archivos de `out/`
  - `errores-semanticos` — casos con errores de tipos y validaciones semánticas
  - `errrores-lex-sint` — casos con errores léxicos y sintácticos
  - `pruebas-requeridas-grupoc` — pruebas obligatorias requeridas para el grupo C
- `out/` — tercetos, tabla de símbolos exportada, `.asm` generado
- `specs/` — especificaciones y bitácoras por etapa
