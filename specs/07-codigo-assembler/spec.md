# Spec — 07-codigo-assembler

**Grupo:** C

**Lenguaje de implementación:** C

**Plataforma destino:** x86-64 (64 bits), NASM assembler (`-f win64`), linkeado mediante GCC (MinGW-w64) en Windows.

## 1. Objetivo

El objetivo de este módulo es recibir el código intermedio (representado como un arreglo lineal de estructuras de **Tercetos**) en conjunto con la **Tabla de Símbolos**, y traducirlo de manera estricta y sistemática a código fuente en lenguaje Assembler x86-64 (arquitectura de 64 bits, formato NASM `-f win64`).

Este código generado debe ser completamente funcional, cumplir con las reglas aritméticas y de forma normal del tipo racional exacto establecidas para el Grupo C, y ser apto para ensamblarse y linkearse mediante GCC en entornos Windows para su ejecución final.

---

## 2. Alcance

Este módulo abarca estrictamente la etapa final de generación de código del compilador. Sus límites operacionales son:

### Lo que SÍ incluye (In-scope)

- La lectura y el procesamiento secuencial de la tabla de tercetos (`tabla_tercetos[]`) y la Tabla de Símbolos (`tabla_simbolos[]`) provistas por las fases anteriores.
- La generación de un archivo de código fuente en Assembler (`salida.asm`) compatible con NASM (x86-64, formato `win64`).
- La emisión de una biblioteca fija de rutinas auxiliares en `.text` para garantizar las reglas de forma normal del racional (simplificación por MCD mediante el algoritmo de Euclides, signo en el numerador, representación única `0/1`), control previo de overflow en 64 bits y aborto ante división por cero en tiempo de ejecución.
- La traducción de operaciones aritméticas racionales (`+`, `-`, `*`, `/`) con simplificación cruzada previa, comparaciones exactas sin punto flotante, operadores lógicos (`and`, `or`), estructuras de control (`si`/`sino` y `mientras`), sentencia de salida (`mostrar`) y llamadas/retornos de funciones sin parámetros (`def`, `retornar`, `principal`).
- El mapeo de variables globales, constantes de cadena y resultados temporales de tercetos bajo el modelo de memoria estática.

### Lo que NO incluye (Out-of-scope)

- Validaciones léxicas, sintácticas o semánticas estáticas (variables no declaradas, división por literal `/0` o llamadas recursivas directas), tareas exclusivas de los analizadores previos.
- Uso de instrucciones de punto flotante (`x87` o `SSE`), prohibidas por la consigna de exactitud racional.
- Soporte para memoria dinámica, asignación en heap, marcos de pila (*stack frames*) para variables locales o funciones recursivas (Decisiones D4 y C.5).

---

## 3. Entradas / Salidas

### Entradas

**Tabla de Tercetos (`tabla_tercetos[]` e `indice_terceto`):**

Arreglo lineal en memoria provisto por el módulo sintáctico/intermedio. Cada celda `[Ti]` contiene:

- `operacion`: Código de la operación a ejecutar (`=`, `+`, `-`, `*`, `/`, `>`, `<`, `>=`, `<=`, `==`, `!=`, `AND`, `OR`, `BF`, `BI`, `MOSTRAR_ID`, `MOSTRAR_STR`, `IR_PRINCIPAL`, `PRINCIPAL`, `FUNC_INICIO`, `FUNC_FIN`, `RETORNAR`, `CALL`).
- `operando1`: Primer operando (identificador, literal racional, referencia a temporal `[Tk]`, o destino de salto en `BI`).
- `operando2`: Segundo operando (identificador, literal racional, referencia a temporal `[Tk]`, o destino de salto en `BF`).

**Tabla de Símbolos (`tabla_simbolos[]` y `cant_simbolos`):**

Estructura con los identificadores globales (`ID`), constantes racionales (`LIT_RACIONAL`) y literales de cadena (`LIT_CADENA`) reconocidos en el código fuente.

### Salidas

- **Archivo de Código Assembler (`salida.asm`):** Archivo de texto fuente con las instrucciones en lenguaje ensamblador (x86-64, formato NASM `win64`) listo para ser ensamblado y linkeado.
- **Artefactos Binarios (`salida.obj` y `salida.exe`):** Generados mediante:
  ```bash
  nasm -f win64 salida.asm -o salida.obj
  gcc salida.obj -o salida.exe
  ```

---

## 4. Reglas y decisiones de diseño

### 4.1. Plataforma y Entorno

El código Assembler generado está diseñado para ejecutarse bajo los siguientes parámetros:

- **Arquitectura:** x86-64 (64 bits).
- **Sintaxis y Formato:** NASM (`-f win64`).
- **Entorno:** Windows, ensamblado con NASM y linkeado mediante GCC (MinGW-w64) para interoperar con el runtime estándar de C (`printf` y `exit`) bajo la convención de llamadas ABI de Microsoft x64 (primeros argumentos en `RCX`, `RDX`, `R8`, *shadow space* de 32 bytes y direccionamiento relativo `[rel etiqueta]`).

### 4.2. Modelo de memoria estática y representación de racionales

El compilador del lenguaje Razio utiliza memoria estática plana, lo que implica que no se utiliza memoria dinámica ni marcos de pila (*stack frames*) dinámicos para la gestión de las variables.

- **Representación de Racionales (32 bits con signo — D1 y D2):** Cada variable racional `x` declarada en la Tabla de Símbolos se almacena en la sección `.data` como un par contiguo de enteros dobles de 32 bits (`dd`), representando numerador y denominador respectivamente (rango $-2.147.483.648$ a $2.147.483.647$).

  ```nasm
  x_num dd 0
  x_den dd 1
  ```

- **Temporales de Tercetos (`T0` a `Tn`):** Cada terceto `i` dispone de su propio par estático `T<i>_num dd 0` y `T<i>_den dd 1` en `.data` para resguardar resultados intermedios sin conflictos de registros en expresiones anidadas.
- **Literales de Cadena y Mensajes del Sistema:** Los literales de texto utilizados en `mostrar` se almacenan en `.data` como cadenas terminadas en byte nulo (`STR_k db "...", 0`), junto con los especificadores de formato (`fmt_rac db "%d/%d", 10, 0`, `fmt_str db "%s", 10, 0`) y los mensajes obligatorios de error en ejecución (`err_ovf` y `err_cero`).

### 4.3. Traducción de operadores, normalización y control previo de overflow

El proceso de traducción recorre secuencialmente la tabla de tercetos desde `0` hasta `indice_terceto - 1`, emitiendo una etiqueta `ETIQ_i:` antes de cada instrucción intermedia.

- **Aritmética en 64 bits con alarma previa de Overflow (Regla C.2):** Los operandos de 32 bits se extienden con signo a registros de 64 bits (`movsxd r12, eax` ... `r15`) para realizar productos cruzados y sumas/restas sin desbordamiento físico de hardware. Antes de reducir a 32 bits, se invoca la rutina `verificar_overflow_64`, que comprueba que `RAX` esté dentro de $[-2.147.483.648, 2.147.483.647]$; si excede el rango, salta a `error_overflow` y aborta con `exit(1)` antes de que ocurra truncamiento.
- **Simplificación Cruzada en `*` y `/` (Regla C.2.9):** Antes de multiplicar $(a/b) \cdot (c/d)$, se invoca `mcd_64_positivo` para reducir cruzadamente $a$ con $d$ y $c$ con $b$, minimizando el crecimiento de los valores intermedios.
- **Forma Normal Obligatoria (Regla C.1):** Tras cada asignación o cálculo aritmético se invoca `normalizar_racional` (entrada: `EAX` = num, `EBX` = den), que verifica `EBX == 0` (abortando en `error_div_cero`), normaliza `0/x` a `0/1`, traslada el signo negativo al numerador si `EBX < 0` y divide ambos por su MCD mediante el algoritmo de Euclides.

**Ejemplo práctico de traducción (para la expresión `c = a + b`):**

- **Tercetos recibidos:**
  - `[T0] (+, a, b)`
  - `[T1] (=, c, [T0])`

- **Traducción generada en Assembler (NASM x64):**

  ```nasm
  ETIQ_0:                         ; Marca el inicio del terceto T0: a + b

    ; Recuperamos el numerador y denominador de a
    mov eax, [rel a_num]        ; Carga el numerador de a en EAX
    mov ebx, [rel a_den]        ; Carga el denominador de a en EBX

    ; Extendemos los valores de 32 a 64 bits
    movsxd r12, eax             ; Copia el numerador de a a R12 (64 bits)
    movsxd r13, ebx             ; Copia el denominador de a a R13 (64 bits)

    ; Recuperamos el numerador y denominador de b
    mov eax, [rel b_num]        ; Carga el numerador de b en EAX
    mov ebx, [rel b_den]        ; Carga el denominador de b en EBX

    ; Extendemos los valores de 32 a 64 bits
    movsxd r14, eax             ; Copia el numerador de b a R14 (64 bits)
    movsxd r15, ebx             ; Copia el denominador de b a R15 (64 bits)

    ; Calculamos el numerador de la suma:
    ; (a_num * b_den) + (b_num * a_den)

    mov rax, r12                ; Carga el numerador de a en RAX
    imul rax, r15               ; Multiplica a_num * b_den
    call verificar_overflow_64  ; Comprueba si hubo desbordamiento

    mov rcx, rax                ; Guarda el primer producto en RCX

    mov rax, r14                ; Carga el numerador de b en RAX
    imul rax, r13               ; Multiplica b_num * a_den
    call verificar_overflow_64  ; Comprueba si hubo desbordamiento

    add rcx, rax                ; Suma los dos productos
    mov rax, rcx                ; Coloca el numerador resultante en RAX
    call verificar_overflow_64  ; Comprueba el resultado de la suma

    push rax                    ; Guarda temporalmente el numerador en la pila

    ; Calculamos el denominador:
    ; a_den * b_den

    mov rax, r13                ; Carga el denominador de a
    imul rax, r15               ; Multiplica a_den * b_den
    call verificar_overflow_64  ; Comprueba si hubo desbordamiento

    mov ebx, eax                ; Guarda el denominador en EBX
    pop rax                     ; Recupera el numerador desde la pila

    call normalizar_racional    ; Simplifica la fracción y normaliza su signo

    ; Guardamos el resultado en el temporal T0
    mov [rel T0_num], eax       ; Guarda el numerador de T0
    mov [rel T0_den], ebx       ; Guarda el denominador de T0


### ETIQ_1:                         ; Marca el inicio del terceto T1: c = [T0]
```
    mov eax, [rel T0_num]       ; Recupera el numerador del resultado
    mov ebx, [rel T0_den]       ; Recupera el denominador del resultado

    call normalizar_racional    ; Asegura que el racional esté normalizado

    mov [rel c_num], eax        ; Guarda el numerador en c
    mov [rel c_den], ebx        ; Guarda el denominador en c
  ```
### 4.4. Manejo de comparaciones exactas y estructuras de control (`si`/`sino` y `mientras`)

Dado que el código intermedio ya resolvió las direcciones de salto mediante *backpatching* (`[Td]`), el generador traduce cada terceto `i` precedido por `ETIQ_i:`.

- **Comparaciones exactas sin punto flotante (Regla C.1.5):** Para `>`, `<`, `>=`, `<=`, `==`, `!=`, se calcula en 64 bits el producto cruzado $num_1 \cdot den_2$ contra $num_2 \cdot den_1$, comparando con `cmp rax, rcx` y almacenando `1` o `0` en `T<i>_num` mediante instrucciones `setg`, `setl`, `setge`, `setle`, `sete` o `setne`.
- **Operadores lógicos (`AND`, `OR`) y saltos (`BF`, `BI`):** `BF` evalúa si el temporal de condición es `0` (`cmp eax, 0` / `je ETIQ_d`) para saltar al bloque `sino` o al final de la estructura; `BI` emite un salto incondicional `jmp ETIQ_d`.

**Ejemplo práctico (`si (a > b) { c = 1; }`):**

- **Tercetos recibidos:**
  - `[T0] (>, a, b)`
  - `[T1] (BF, [T0], [T3])`
  - `[T2] (=, c, 1)`
  - `[T3] ...`

- **Traducción generada en Assembler (NASM x64):**

  ```nasm
  ETIQ_0: ; Terceto [T0] (>, a, b) - Comparar si a es mayor que b

    ; Recuperar numerador y denominador de la variable a
    movsxd r12, dword [rel a_num] ; Cargar numerador de a en R12 (64 bits)
    movsxd r13, dword [rel a_den] ; Cargar denominador de a en R13 (64 bits)

    ; Recuperar numerador y denominador de la variable b
    movsxd r14, dword [rel b_num] ; Cargar numerador de b en R14 (64 bits)
    movsxd r15, dword [rel b_den] ; Cargar denominador de b en R15 (64 bits)

    ; Realizar el producto cruzado para comparar los racionales
    mov rax, r12                   ; Copiar numerador de a
    imul rax, r15                  ; Multiplicar numerador de a por denominador de b

    mov rcx, r14                   ; Copiar numerador de b
    imul rcx, r13                  ; Multiplicar numerador de b por denominador de a

    ; Comparar los resultados de los productos cruzados
    cmp rax, rcx                   ; Comparar ambos valores
    setg al                        ; Guardar 1 si a > b; 0 si no se cumple

    ; Guardar el resultado de la comparación en el temporal T0
    movzx eax, al                  ; Extender el resultado de 8 a 32 bits
    mov [rel T0_num], eax          ; Guardar 1 (verdadero) o 0 (falso)
    mov dword [rel T0_den], 1      ; El denominador del resultado lógico es 1


ETIQ_1: ; Terceto [T1] (BF, [T0], [T3]) - Saltar si la condición es falsa

    mov eax, [rel T0_num]          ; Recuperar el resultado de la comparación
    cmp eax, 0                     ; Comprobar si el resultado es falso (0)
    je ETIQ_3                      ; Si es falso, saltar al final del bloque


ETIQ_2: ; Terceto [T2] (=, c, 1) - Asignar 1 a la variable c

    mov eax, 1                     ; Cargar numerador 1
    mov ebx, 1                     ; Cargar denominador 1
    call normalizar_racional       ; Normalizar el racional 1/1
    mov [rel c_num], eax           ; Guardar numerador en c
    mov [rel c_den], ebx           ; Guardar denominador en c


ETIQ_3: ; Fin del bloque si
  ```

### 4.5. Generación de la sentencia de salida (`mostrar`)

La sentencia `mostrar` se resuelve mediante interoperabilidad con `printf` de la biblioteca estándar de C (MinGW-w64), respetando el *shadow space* de 32 bytes de Windows x64.

- **Impresión de Variables Racionales (`MOSTRAR_ID`):** Carga `<id>_num` en `ECX` y `<id>_den` en `EDX` e invoca la rutina auxiliar `imprimir_racional`, la cual llama a `printf("%d/%d\n", num, den)`.
- **Impresión de Cadenas Literales (`MOSTRAR_STR`):** Carga la dirección de `fmt_str` (`"%s\n"`) en `RCX` y la etiqueta `STR_k` correspondiente en `RDX`, invocando `printf`.

```nasm
; Ejemplo para mostrar("Resultado:", c);
ETIQ_4: ; [MOSTRAR_STR, Resultado:, -]
    lea rcx, [rel fmt_str]
    lea rdx, [rel STR_0]
    call printf

ETIQ_5: ; [MOSTRAR_ID, c, -]
    mov ecx, [rel c_num]
    mov edx, [rel c_den]
    call imprimir_racional
```

### 4.6. Manejo de funciones, punto de entrada (`principal`) y retornos (Regla C.5)

El lenguaje Razio admite funciones sin parámetros ni variables locales que operan sobre memoria estática global y pueden retornar un valor racional.

- **Salto al punto de entrada (`IR_PRINCIPAL` / `PRINCIPAL`):** Tras ejecutar las asignaciones iniciales de variables globales, `jmp ETIQ_PRINCIPAL` salta por encima de las definiciones de funciones ubicadas antes de `principal()`, evitando su ejecución accidental al iniciar el programa.
- **Definición y Retorno (`FUNC_INICIO`, `RETORNAR`, `FUNC_FIN`):** Cada función `def f()` emite la etiqueta `FUNC_f:` y reserva 40 bytes en pila (`sub rsp, 40`). Al procesar `retornar(val)`, normaliza el racional dejando el numerador en `EAX` y el denominador en `EBX`, restaura la pila (`add rsp, 40`) y ejecuta `ret`.
- **Invocación (`CALL`):** Emite `call FUNC_f` y guarda el par devuelto (`EAX`, `EBX`) en el temporal `T<i>_num` y `T<i>_den` correspondiente.

---

## 5. Criterios de aceptación

Para considerar que el módulo de Generación de Código Assembler es correcto y funcional, se deben cumplir los siguientes criterios de validación:

1. **Consumo íntegro de la Tabla de Tercetos:** El módulo debe procesar secuencialmente la totalidad de los tercetos generados sin omitir instrucciones ni desalinear la pila x64 (`RSP`).
2. **Generación sintáctica correcta del `.asm`:** El archivo `salida.asm` generado debe cumplir estrictamente con la sintaxis de NASM para x86-64 (`-f win64`), permitiendo su ensamblaje y linkeo limpio con `nasm` y `gcc` sin errores ni advertencias.
3. **Cumplimiento de forma normal y simplificación (Regla C.1):** Todo racional mostrado por consola debe estar simplificado por su MCD (ej. `1/2 + 1/3` muestra `5/6`; `2/4` muestra `1/2`), con denominador positivo y el cero representado como `0/1`.
4. **Detección previa de overflow y división por cero (Reglas C.1.3 y C.2):** Ante operaciones que superen el rango de 32 bits con signo ($[-2.147.483.648, 2.147.483.647]$) o
