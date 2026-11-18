# Spec — 07-codigo-assembler

**Grupo:** C  
**Lenguaje de implementación:** C  
**Plataforma destino:** x86-64 (64 bits), NASM assembler (`-f win64`), linkeado mediante GCC (MinGW-w64) en Windows.

---

## 1. Objetivo
El objetivo de este módulo es transformar los tercetos generados por el compilador, junto con la información de la Tabla de Símbolos, en código Assembler para Windows de 64 bits (x86-64), utilizando NASM.

El código generado debe poder realizar las operaciones del lenguaje Razio, respetando las reglas de los números racionales, como la simplificación de fracciones y el control de errores. Finalmente, debe poder ensamblarse y enlazarse mediante GCC para obtener un archivo ejecutable en Windows.

---

## 2. Alcance

Este módulo abarca estrictamente la etapa final de generación de código del compilador. Sus límites operacionales son:

### 2.1. Lo que SÍ incluye (In-scope)

- Leer y recorrer los tercetos (tabla_tercetos[]) y la Tabla de Símbolos (tabla_simbolos[]) generados en las etapas anteriores del compilador.
- Generar el archivo salida.asm con las instrucciones necesarias para NASM, utilizando la arquitectura de 64 bits para Windows.
- Incorporar rutinas auxiliares para trabajar correctamente con números racionales. Esto incluye simplificar las fracciones mediante el algoritmo de Euclides, mantener el signo en el numerador y representar el cero como 0/1.
- Controlar posibles desbordamientos numéricos (overflow) y evitar divisiones por cero durante la ejecución del programa.
- Traducir las operaciones aritméticas (+, -, *, /), las comparaciones, los operadores lógicos (AND, OR), las estructuras de control (si, sino, mientras), la instrucción mostrar y las llamadas y retornos de funciones sin parámetros.
- Reservar espacio en memoria para las variables globales, las cadenas de texto y los resultados intermedios de los tercetos.

### 2.2. Lo que NO incluye (Out-of-scope)

- Realizar validaciones léxicas, sintácticas o semánticas, ya que estas corresponden a las etapas anteriores del compilador.

---

## 3. Entradas / Salidas

### 3.1. Entradas

**Tabla de Tercetos (`tabla_tercetos[]` e `indice_terceto`):**

Arreglo lineal en memoria provisto por el módulo sintáctico/intermedio. Cada celda `[Ti]` contiene:

- `operacion`: Código de la operación a ejecutar (`=`, `+`, `-`, `*`, `/`, `>`, `<`, `>=`, `<=`, `==`, `!=`, `AND`, `OR`, `BF`, `BI`, `MOSTRAR_ID`, `MOSTRAR_STR`, `IR_PRINCIPAL`, `PRINCIPAL`, `FUNC_INICIO`, `FUNC_FIN`, `RETORNAR`, `CALL`).
- `operando1`: Primer operando (identificador, literal racional, referencia a temporal `[Tk]`, o destino de salto en `BI`).
- `operando2`: Segundo operando (identificador, literal racional, referencia a temporal `[Tk]`, o destino de salto en `BF`).

**Tabla de Símbolos (`tabla_simbolos[]` y `cant_simbolos`):**

Estructura con los identificadores globales (`ID`), constantes racionales (`LIT_RACIONAL`) y literales de cadena (`LIT_CADENA`) reconocidos en el código fuente.

### 3.2. Salidas

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
- **Entorno:** Windows, utilizando NASM para ensamblar el código y GCC (MinGW-w64) para generar el ejecutable.
- **Comunicación con C:** Se utilizan las funciones printf y exit de la biblioteca estándar de C.
- **Convención de llamadas:** Se utiliza la convención Microsoft x64, donde los primeros argumentos se pasan mediante los registros RCX, RDX y R8, y se reserva un espacio de 32 bytes (shadow space) para las llamadas a funciones. También se utiliza el direccionamiento relativo ([rel etiqueta]) para acceder a los datos.


### 4.2. Modelo de memoria estática y representación de racionales

El compilador utiliza un modelo de memoria estática, es decir, reserva el espacio necesario para las variables y los resultados desde el inicio del programa. No utiliza memoria dinámica para almacenar estos datos.

- **Representación de Racionales (32 bits con signo):** Cada variable racional declarada se almacena en la sección .data mediante dos valores enteros de 32 bits (dd): uno para el numerador y otro para el denominador. Ambos tienen signo y permiten representar valores entre -2.147.483.648 y 2.147.483.647.
  Por ejemplo, para una variable x se reserva:
  
```nasm
x_num dd 0
x_den dd 1
```
x_num almacena el numerador y x_den el denominador. Inicialmente, la variable representa 0/1.

- **Temporales de Tercetos (`T0` a `Tn`):** Cada terceto tiene su propio espacio de memoria para guardar el resultado de las operaciones. Se utilizan dos valores, uno para el numerador y otro para el denominador, identificados con el número del terceto.
 Por ejemplo:
```nasm
T0_num dd 0
T0_den dd 1
```
- **Literales de Cadena y Mensajes del Sistema:** Los textos utilizados por la instrucción mostrar se almacenan en la sección .data, terminados en un byte nulo (0). También se guardan los formatos necesarios para mostrar racionales y cadenas de texto, junto con los mensajes que se utilizan para informar errores durante la ejecución, como el desbordamiento numérico (err_ovf) y la división por cero (err_cero).
```
section .data

STR_0 db "Hola mundo", 0       ; Texto utilizado por mostrar.
fmt_str db "%s", 10, 0         ; Formato para mostrar cadenas.
fmt_rac db "%d/%d", 10, 0      ; Formato para mostrar racionales.
err_ovf db "Error: overflow", 0 ; Mensaje de desbordamiento.
err_cero db "Error: division por cero", 0 ; Mensaje de división por cero.
```

### 4.3. Traducción de operadores, normalización y control previo de overflow

El generador recorre la tabla de tercetos en orden, desde el primero hasta el último. Para cada terceto genera las instrucciones Assembler correspondientes y coloca una etiqueta (ETIQ_i) que permite identificar su posición dentro del código.
- **Aritmética en 64 bits con alarma previa de Overflow (Regla C.2):** Para realizar las operaciones aritméticas, los valores de 32 bits se extienden a registros de 64 bits, lo que permite trabajar con resultados intermedios más grandes. Antes de guardar el resultado, se ejecuta la función verificar_overflow_64, que comprueba que el valor esté dentro del rango permitido para un entero de 32 bits. Si se supera ese límite, se muestra un mensaje de error mediante error_overflow y se detiene la ejecución del programa con exit(1).
- **Simplificación Cruzada en `*` y `/` (Regla C.2.9):** Antes de realizar operaciones, se simplifican las fracciones cruzando el numerador de una con el denominador de la otra. Esto permite reducir los valores intermedios y disminuir el riesgo de overflow.
- **Forma Normal Obligatoria (Regla C.1):** Después de cada asignación u operación aritmética, se ejecuta la función normalizar_racional, que recibe el numerador en EAX y el denominador en EBX. Esta función verifica que el denominador no sea cero y, en ese caso, informa el error mediante error_div_cero y detiene la ejecución. Además, representa el cero como 0/1, coloca el signo negativo en el numerador cuando corresponde y simplifica la fracción utilizando el máximo común divisor (MCD), calculado mediante el algoritmo de Euclides.

#### Ejemplo práctico de traducción: `c = a + b`

**Tercetos recibidos:**

- `[T0] (+, a, b)`
- `[T1] (=, c, [T0])`

**Traducción generada en Assembler (NASM x64):**

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

ETIQ_1:                         ; Marca el inicio del terceto T1: c = [T0]

    mov eax, [rel T0_num]       ; Recupera el numerador del resultado
    mov ebx, [rel T0_den]       ; Recupera el denominador del resultado

    call normalizar_racional    ; Asegura que el racional esté normalizado

    mov [rel c_num], eax        ; Guarda el numerador en c
    mov [rel c_den], ebx        ; Guarda el denominador en c
```

### 4.4. Manejo de comparaciones exactas y estructuras de control (`si`/`sino` y `mientras`)

El generador recorre los tercetos en orden y utiliza las etiquetas ETIQ_i para identificar la posición de cada instrucción. Las direcciones de los saltos ya fueron establecidas durante la generación del código intermedio mediante el mecanismo de backpatching.

- **Comparaciones exactas sin punto flotante (Regla C.1.5):** Para realizar las comparaciones (>, <, >=, <=, ==, !=), se utiliza el producto cruzado entre los numeradores y denominadores de los racionales. Los cálculos se realizan en 64 bits para comparar los valores sin convertirlos a punto flotante. El resultado se guarda en el temporal correspondiente como 1 si la condición es verdadera o 0 si es falsa.

- **Operadores lógicos (`AND`, `OR`) y saltos (`BF`, `BI`):** Los operadores AND y OR permiten combinar condiciones lógicas y guardar el resultado en un temporal. El terceto BF comprueba si el resultado de una condición es 0; si lo es, realiza un salto hacia la etiqueta indicada. Se utiliza para controlar el flujo de las estructuras si y mientras. Por otro lado, BI realiza un salto incondicional mediante jmp, permitiendo continuar la ejecución desde la etiqueta correspondiente.

#### Ejemplo práctico: `si (a > b) { c = 1; }`

**Tercetos recibidos:**

- `[T0] (>, a, b)`
- `[T1] (BF, [T0], [T3])`
- `[T2] (=, c, 1)`
- `[T3] ...`

**Traducción generada en Assembler (NASM x64):**

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

La instrucción mostrar permite imprimir variables racionales y cadenas de texto en pantalla. Para hacerlo, el código generado utiliza la función printf de la biblioteca estándar de C.

- **Impresión de Variables Racionales (`MOSTRAR_ID`):** El generador obtiene el numerador y el denominador de la variable y los pasa a la función auxiliar imprimir_racional. Esta función utiliza printf para mostrar la fracción con el formato numerador/denominador.
- **Impresión de Cadenas Literales (`MOSTRAR_STR`):** El generador obtiene la dirección de la cadena almacenada en STR_k y la pasa a printf, utilizando el formato fmt_str ("%s\n"). De esta manera, se muestra el texto en pantalla.

```nasm
; Ejemplo para mostrar("Resultado:", c);

ETIQ_4:                         ; [MOSTRAR_STR, "Resultado:", -]
    lea rcx, [rel fmt_str]      ; Carga en RCX la dirección del formato de impresión (%s).
    lea rdx, [rel STR_0]        ; Carga en RDX la dirección de la cadena "Resultado:".
    call printf                 ; Llama a printf para mostrar el texto en consola.

ETIQ_5:                         ; [MOSTRAR_ID, c, -]
    mov ecx, [rel c_num]        ; Carga el numerador de la variable c en ECX.
    mov edx, [rel c_den]        ; Carga el denominador de la variable c en EDX.
    call imprimir_racional     ; Llama a la rutina que imprime c en formato numerador/denominador.
```

### 4.6. Manejo de funciones, punto de entrada (`principal`) y retornos (Regla C.5)

El lenguaje Razio admite funciones sin parámetros ni variables locales que operan sobre memoria estática global y pueden retornar un valor racional.

- **Salto al punto de entrada (`IR_PRINCIPAL` / `PRINCIPAL`):** Primero se realizan las asignaciones iniciales de las variables globales. Luego, se ejecuta jmp ETIQ_PRINCIPAL, que permite saltar directamente al comienzo de principal(), evitando que se ejecuten las definiciones de las funciones que aparecen antes.
- **Definición y Retorno (`FUNC_INICIO`, `RETORNAR`, `FUNC_FIN`):** Al comenzar una función, FUNC_INICIO genera una etiqueta FUNC_f que identifica dónde empieza su código y reserva 40 bytes en la pila. Cuando se ejecuta retornar(val), se prepara el valor racional que se quiere devolver, guardando el numerador en EAX y el denominador en EBX. Después, se libera el espacio reservado en la pila y se ejecuta ret para volver al lugar desde donde se llamó a la función. Por último, FUNC_FIN marca el final del código de la función.
- **Invocación (`CALL`):** Cuando se llama a una función, se genera la instrucción call FUNC_f, que transfiere la ejecución a su código. Una vez que la función devuelve el resultado, el numerador (EAX) y el denominador (EBX) se guardan en el temporal correspondiente (T<i>_num y T<i>_den), para poder utilizar ese valor en otras operaciones.
- **Efectos laterales:** Como Razio no utiliza variables locales, las funciones trabajan directamente con las variables globales almacenadas en la sección .data. Por lo tanto, si una función modifica una variable, ese cambio se mantiene después de que termina su ejecución y puede ser utilizado por principal() o por cualquier otra función.

---

## 5. Criterios de aceptación

Para considerar que el módulo de Generación de Código Assembler es correcto y funcional, se deben cumplir los siguientes criterios de validación:

1. **Procesamiento de la Tabla de Tercetos:** El generador debe recorrer y procesar todos los tercetos en el orden correspondiente, sin omitir instrucciones ni producir errores en el manejo de la pila.

2. **Generación correcta del `salida.asm`:** El archivo salida.asm debe respetar la sintaxis de NASM para Windows de 64 bits (-f win64). Además, debe poder ensamblarse y enlazarse mediante nasm y gcc sin errores.

3. **Cumplimiento de forma normal y simplificación:** Los números racionales deben mostrarse siempre simplificados, con el denominador positivo y el cero representado como 0/1. Por ejemplo, 1/2 + 1/3 debe mostrar 5/6, mientras que 2/4 debe mostrar 1/2.

4. **Detección previa de overflow y división por cero:** El programa debe detectar cuando una operación supera el rango permitido para un entero de 32 bits o cuando se intenta dividir por cero. En esos casos, debe mostrar el mensaje de error correspondiente y detener la ejecución con un código de salida distinto de cero.

5. **Ejecución de pruebas obligatoria:** El ejecutable generado debe superar las pruebas obligatorias: tres programas generales que evalúan estructuras de control anidadas, lógica booleana y funciones con retorno, además de los cuatro casos específicos correspondientes al Grupo C.
