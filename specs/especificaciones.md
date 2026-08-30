## 1. Alfabeto

Σ = L ∪ D ∪ O ∪ R ∪ S ∪ B

| Clase | Caracteres |
| :---: | :--- |
| **L** | `a – z`, `A – Z` |
| **D** | `0 – 9` |
| **O** | `+`, `-`, `*`, `/`, `=`, `<`, `>` |
| **R** | `$` (utilizado para delimitar racionales) |
| **S** | `(`, `)`, `{`, `}`, `;`, `:`, `,`, `"` |
| **B** | espacio, tabulación (`\t`), salto de línea (`\n`), retorno de carro (`\r`) |
| **OTRO** | cualquier otro carácter → error léxico |

**Cualquier carácter presente en el archivo .txt que no pertenezca a esta unión de conjuntos (por ejemplo: @, #, ~, ?, _, etc.) 
será rechazado por el analizador léxico, y generará un error léxico con el número de línea correspondiente**

## 2.PALABRAS RESERVADAS
def · principal · entero · racional · cadena · si · sino · mientras · mostrar · retornar

## 3. Tabla de tokens

| Código | Token | Lexema |
| :---: | :--- | :--- |
| **257** | `PR_DEF` | `def` |
| **258** | `PR_RACIONAL` | `racional` |
| **259** | `PR_SI` | `si` |
| **260** | `PR_SINO` | `sino` |
| **261** | `PR_MIENTRAS` | `mientras` |
| **262** | `PR_MOSTRAR` | `mostrar` |
| **263** | `PR_PRINCIPAL` | `principal` |
| **264** | `ID` | Identificador (`a`, `acum`) |
| **265** | `LIT_RACIONAL` | Constante racional (`$3/4$`) |
| **266** | `LIT_ENTERO` | Constante entera (`5`) |
| **267** | `LIT_CADENA` | Cadena de texto (`"Resultado:"`) |
| **268** | `OP_ASIG` | `=` |
| **269** | `OP_SUMA` | `+` |
| **270** | `OP_RESTA` | `-` |
| **271** | `OP_MULT` | `*` |
| **272** | `OP_DIV` | `/` |
| **273** | `COMP_MAYOR` | `>` |
| **274** | `COMP_MENOR` | `<` |
| **275** | `COMP_MAYOR_IGUAL` | `>=` |
| **276** | `COMP_MENOR_IGUAL` | `<=` |
| **277** | `COMP_IGUAL` | `==` |
| **278** | `COMP_DISTINTO` | `!=` |
| **279** | `OP_AND` | `and` |
| **280** | `OP_OR` | `or` |
| — | Literales | `(`, `)`, `{`, `}`, `;`, `:`, `,` se devuelven como su propio carácter |





## 4.PROGRAMA DE EJEMPLO
// Muestra el uso de identificadores, palabras reservadas,
// racionales explícitos, cadenas y estructuras básicas



def principal() {
    // Declaración de variables y racionales explícitos
    racional mi_fraccion;
    entero contador;
    cadena mensaje;

    mi_fraccion = $3/4$;
    contador = 1;
    mensaje = "Procesando datos...";

    // Estructura condicional y comparación
    si (contador <= 10) {
        mostrar(mensaje);
    } sino {
        mostrar("Límite superado");
    }

    // Bucles y operadores relacionales
    mientras (contador < 5) {
        contador = contador + 1;
    }

    retornar 0;
}











