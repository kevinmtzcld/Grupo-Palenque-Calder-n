# Spec — 06-codigo-intermedio

## 1. Objetivo
Esta etapa permite transformar las instrucciones del programa en una serie de operaciones más simples y ordenadas. Los tercetos sirven como paso intermedio entre el análisis del código fuente y la posterior generación del código ensamblador.

## 2. Alcance
Definición de qué tareas se van a realizar en esta etapa del compilador y cuáles quedan fuera.

**Incluye:**

- Definición de la estructura utilizada en C para almacenar los tercetos.
- Generación de tercetos para operaciones aritméticas (`+`, `-`, `*`, `/`) y asignaciones (`=`).
- Representación de las estructuras condicionales (`si` / `sino`) e iterativas (`mientras`).
- Generación de instrucciones para la función de salida `mostrar`.
- Resolución de los destinos de los saltos mediante la técnica de *Backpatching*.
- Exportación de los tercetos generados a un archivo llamado `tercetos.txt`.

**No incluye:**

- Generación del código ensamblador o del código ejecutable final.
- Optimización de los tercetos, como simplificar operaciones o eliminar instrucciones innecesarias.

## 3. Entradas y salidas

**Entradas:**

**Código fuente**: las instrucciones de Razio que Bison reconoce utilizando las reglas gramaticales definidas en sintactico.y.

**Datos de las instrucciones**: los valores necesarios para representar las operaciones, como nombres de variables, números y expresiones matemáticas.

**Tabla de Símbolos**: la información que permite verificar si las variables utilizadas fueron declaradas previamente.

**Salidas:**

**Tabla de tercetos en memoria:** las operaciones se almacenan en tabla_tercetos[] a medida que se procesa el programa.

**Archivo tercetos.txt:** al finalizar el proceso, los tercetos se guardan en un archivo de texto, donde cada uno representa una operación mediante un operador y dos operandos.

## 4. Reglas y decisiones de diseño

### 4.1. Estructura de los tercetos y almacenamiento

Cada terceto representa una operación y contiene tres campos: la operación y sus dos operandos.

Para almacenar esta información se utiliza la siguiente estructura en C:

```c
typedef struct {
    char operacion[15]; // Operación a realizar (+, =, BF, BI, etc.)
    char operando1[50]; // Primer operando
    char operando2[50]; // Segundo operando
} Terceto;

Terceto tabla_tercetos[1000];
int indice_terceto = 0;
```

La tabla permite guardar los tercetos a medida que se reconocen las distintas instrucciones del programa.

### 4.2. Uso de referencias en lugar de variables temporales

No se crean variables adicionales para almacenar los resultados de las operaciones intermedias.

En su lugar, cada resultado se identifica mediante la posición del terceto que lo generó. Por ejemplo, `[T0]` representa el resultado del primer terceto y puede utilizarse en operaciones posteriores.

### 4.3. Integración con Bison

La generación de tercetos se realiza dentro de las acciones asociadas a las reglas gramaticales del archivo `sintactico.y`.

Cuando Bison reconoce una expresión u otra instrucción del lenguaje, se ejecuta la acción correspondiente y se genera el terceto necesario.

Para trabajar con los valores de cada regla se utilizan las referencias `$1`, `$2`, `$3`, etc., que permiten acceder a sus distintos elementos. Por otro lado, `$$` permite guardar el resultado de la regla completa.

De esta manera, las referencias a los tercetos generados pueden utilizarse en otras expresiones e instrucciones.

### 4.4. Manejo de los tipos de datos

En Razio se utiliza el tipo de dato `racional` para las variables.

Por este motivo, las operaciones aritméticas trabajan con valores de ese tipo. Durante la generación de tercetos también se comprueba, mediante la Tabla de Símbolos, que las variables utilizadas hayan sido declaradas previamente.

### 4.5. Operadores disponibles

Los tercetos contemplan las siguientes operaciones:

- **Aritméticas y asignación:** `+`, `-`, `*`, `/`, `=`.
- **Comparaciones:** `>`, `<`, `>=`, `<=`, `==`, `!=`.
- **Saltos:**
  - `BF`: permite saltar a otra posición cuando una condición es falsa.
  - `BI`: permite saltar a otra posición sin evaluar ninguna condición.
- **Entrada y salida:** `MOSTRAR`.

### 4.6. Manejo de saltos (Backpatching)

Para completar los saltos incompletos se necesitará una función que modifique las posiciones 
correspondientes, ya sea la primera (BI) o la segunda posicion (BF) del terceto.
Para guardar el indice de los tercetos incompletos se necesitará una pila "pila_saltos"
que apilará los indices de tercetos incompletos y desapilará los indices para completar
los tercetos incompletos

### 4.7. Creación de Tercetos
-Para las reglas como asignacion, expresión OP_SUMA termino, 
expresion OP_RESTA termino, termino OP_MULT factor, termino OP_DIV factor
Es cuando se creará el terceto correspondiente a la operación
usando la funcion "crear_terceto" pasandole la operación, el primer operando $1
y el segundo operando $3, esa función devolverá el indice del terceto creado.
y en el NO TERMINAL se guardará una referencia al terceto creado [Ti] para usarlo
en operaciones posteriores

Ejemplos: 
crear_terceto("+", $1, $3)
crear_terceto("/", $1, $3)

### 4.7. MANEJO DE SALTOS (SI/SINO)


Regla marca_si bloque:

Al reconocer la condición del SI, se genera un terceto BF (salto si es falso) con destino incompleto (?), ya que todavía no se conoce dónde debe saltar. Su índice se guarda en la pila de saltos.

Al finalizar el bloque del SI, se completa el destino del BF mediante modificar_terceto, indicando la posición siguiente al bloque.

Regla marca_si bloque PR_SINO:

Al reconocer la condición, se genera un terceto BF con destino incompleto.

Al reconocer el SINO, se completa el destino del BF, indicando dónde comienza el bloque SINO, ya que allí debe saltar si la condición es falsa.

Además, se genera un terceto BI (salto incondicional) con destino incompleto, para evitar que se ejecute el bloque SINO cuando la condición sea verdadera. Su índice también se guarda en la pila.

Regla bloque:

Al finalizar el bloque SINO, se completa el destino del terceto BI, indicando la posición siguiente al bloque, para continuar la ejecución.


### 4.7. MANEJO DE SALTOS (MIENTRAS)


Para implementar un MIENTRAS se necesitan dos saltos:
- BF: se genera al reconocer la condición, con destino incompleto. Si la condición es falsa, debe saltar fuera del ciclo.
- BI: se genera al finalizar el bloque del ciclo. Su destino es el comienzo de la evaluación de la condición, para repetir la iteración.
Al reconocer el MIENTRAS, se guarda la posición del terceto donde comienza la evaluación de la condición.

Al finalizar el bloque, se genera el BI utilizando esa posición como destino. Luego, se completa el BF con la posición siguiente al ciclo, que indica dónde debe continuar la ejecución cuando la condición sea falsa.

Los índices de los saltos pendientes se almacenan en la pila para poder completarlos posteriormente.

### 4.7. MANEJO DE AND Y OR
La implementación se realizó modificando la gramática mediante una jerarquía de reglas (condicion, 
cond_or, cond_and y cond_factor) que permite procesar las condiciones desde sus componentes más 
simples hasta formar la expresión completa, se pasan los resultados desde abajo hacia arriba.
primero, cond_factor reconoce las comparaciones individuales o las condiciones entre paréntesis 
y copia sus referencias a los tercetos mediante strcpy(). Luego, cond_and recibe esos resultados y,
cuando encuentra un operador AND, genera un nuevo terceto mediante crear_terceto("AND", $1, $3),
donde $1 y $3 representan las referencias de las condiciones izquierda y derecha. Mediante sprintf()
guarda la referencia al nuevo terceto ([Tn]) para transmitirla al nivel superior. después, 
cond_or realiza el mismo procedimiento cuando encuentra un operador OR, utilizando los resultados 
recibidos de cond_and para generar el terceto correspondiente. Esta separación de reglas permite 
respetar la prioridad de los operadores, ya que AND tiene prioridad sobre OR. Finalmente, condicion
recibe y transmite la referencia al terceto que representa toda la expresión lógica, para que pueda 
ser utilizada por el terceto BF en la evaluación de la condición y la realización del salto correspondiente.


























## 5. Criterios de aceptación

Para considerar que esta etapa funciona correctamente, se deben cumplir las siguientes condiciones:

1. **Orden correcto de las operaciones:** los tercetos deben generarse respetando la prioridad de los operadores. Por ejemplo, en `x = a + b * c`, primero debe generarse la multiplicación y después la suma.

2. **Formato del archivo:** cuando el análisis del programa finaliza correctamente, se debe generar el archivo `tercetos.txt`, con cada terceto en una línea y siguiendo el formato `[T_] (op, op1, op2)`.

3. **Saltos correctamente resueltos:** las instrucciones `BF` y `BI` deben tener indicado un destino válido dentro de la tabla de tercetos.

4. **Control de errores:** si el programa contiene errores léxicos o sintácticos (`error_lexico != 0` o `error_sintactico != 0`), no se debe generar ni exportar el archivo de tercetos.

