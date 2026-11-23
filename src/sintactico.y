%{
/* ==========================================
   NIVEL 1: DECLARACIONES EN C
   ========================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// 12/11

#include <stdlib.h>

/* Elementos externos que vienen del módulo léxico y entorno C */
extern int yylex(void);
extern int num_linea;
extern int error_lexico;
extern char buffer[];
extern FILE *inputFile;
extern void exportarTS(void);
extern void generar_assembler(void);

int error_sintactico = 0; /* <-- AGREGAR ESTA BANDERA */




// 15-11
int error_semantico = 0;

/* Tabla auxiliar de variables declaradas (D3: Declaración obligatoria) */
char vars_declaradas[500][50];
int cant_vars_declaradas = 0;
char funcion_actual[50] = "";

void registrar_variable(const char *nombre) {
    for (int i = 0; i < cant_vars_declaradas; i++) {
        if (strcmp(vars_declaradas[i], nombre) == 0) {
            printf("\n[ERROR SEMANTICO - Linea %d]: Variable '%s' redeclarada.\n", num_linea, nombre);
            error_semantico = 1;
            return;
        }
    }
    strcpy(vars_declaradas[cant_vars_declaradas++], nombre);
}

void verificar_variable(const char *nombre) {
    for (int i = 0; i < cant_vars_declaradas; i++) {
        if (strcmp(vars_declaradas[i], nombre) == 0) return;
    }
    printf("\n[ERROR SEMANTICO - Linea %d]: Variable '%s' no declarada.\n", num_linea, nombre);
    error_semantico = 1;
}


void yyerror(const char *mensaje);





//--------------------------TERCETO BASE----------------------------------------------------------------------------------------------------------------------

/* 1. Definición del tipo de dato Terceto */
typedef struct {
    char operacion[15];
    char operando1[50];
    char operando2[50];
} Terceto;

/* 2. Arreglo global y contador */
Terceto tabla_tercetos[1000]; // Almacena los tercetos generados durante el análisis sintáctico.
int indice_terceto = 0;       // Indica la posición donde se guardará el próximo terceto.

// 3.FUNCIÓN CREAR TERCETOS
int crear_terceto(const char *op, const char *op1, const char *op2) {

    strcpy(tabla_tercetos[indice_terceto].operacion, op);   // Guarda el operador (+, -, *, /).
    strcpy(tabla_tercetos[indice_terceto].operando1, op1); // Guarda el primer operando.
    strcpy(tabla_tercetos[indice_terceto].operando2, op2); // Guarda el segundo operando.

    indice_terceto++; // Avanza a la siguiente posición para guardar el próximo terceto.

    return indice_terceto - 1; // Devuelve el índice del terceto que acaba de crear.
}

/* 4. Función para guardar los tercetos en el archivo tercetos.txt */
void exportar_tercetos(void) {
    FILE *arch = fopen("tercetos.txt", "w");
    if (!arch) {
        printf("Error al crear el archivo tercetos.txt\n");
        return;
    }
    
    for (int i = 0; i < indice_terceto; i++) {
        fprintf(arch, "[T%d] (%s, %s, %s)\n", 
                i, 
                tabla_tercetos[i].operacion, 
                tabla_tercetos[i].operando1, 
                tabla_tercetos[i].operando2);
    }
    
    fclose(arch);
    printf("[EXITO] Archivo 'tercetos.txt' generado correctamente.\n");
}
//--------------------------TERCETO BASE----------------------------------------------------------------------------------------------------------------------






//--------------------------TERCETO MANEJO DE SALTOS----------------------------------------------------------------------------------------------------------------------

//BF
/* 5. Función para modificar un terceto existente (Backpatching) */
void modificar_terceto(int indice, const char *nuevo_op2) {
    // Modifica el segundo operando de un terceto ya generado.
    // Se utiliza para completar el destino de un salto cuando ya se conoce
    // a qué terceto debe dirigirse
    strcpy(tabla_tercetos[indice].operando2, nuevo_op2);	//RELLENA LA POSICION DE SALTO AL BLOQUE SINO
}

//BI
/* Modifica el primer operando de un terceto (para saltos incondicionales BI) */
void modificar_terceto_op1(int indice, const char *nuevo_op1) {
    strcpy(tabla_tercetos[indice].operando1, nuevo_op1);
}




/* 6. Pila auxiliar para guardar las posiciones de los saltos pendientes */
int pila_saltos[100]; // Almacena los índices de los tercetos de salto pendientes.
int tope_pila = 0;    // Indica la próxima posición libre de la pila.

/* Guarda la posición de un terceto de salto pendiente en la pila. */
void apilar(int pos_terceto) {
    if (tope_pila < 100) {
        pila_saltos[tope_pila++] = pos_terceto;
    } else {
        printf("[ERROR] Desbordamiento de la pila de saltos\n");
    }
}

/* Recupera y elimina de la pila la última posición de salto guardada. */
int desapilar(void) {
    if (tope_pila > 0) {
        return pila_saltos[--tope_pila];
    }
    printf("[ERROR] Pila de saltos vacía\n");
    return -1;
}
//--------------------------TERCETO MANEJO DE SALTOS----------------------------------------------------------------------------------------------------------------------





%}



/* ==========================================
   NIVEL 2: TOKENS Y PRECEDENCIA DE BISON
   ========================================== */
   
   
//---------------------------------------PERMITE PASAR CONTENIDOS ENTRE REGLAS---------------------------------------------------------------------------------------------------------
%code requires {
    // 1. ListaSaltos: es simplemente un array de números enteros.
    //    Guarda los ÍNDICES de los tercetos de salto que tienen un "?" pendiente.
    //    Ejemplo: si [T1] y [T3] son saltos pendientes, list = {1, 3} y count = 2.
    typedef struct {
        int list[100]; // Los números de terceto a rellenar (hasta 100 saltos)
        int count;     // Cuántos saltos hay guardados actualmente en el array
    } ListaSaltos;

    // 2. CondAttr ("Condition Attributes" = Atributos de una Condición):
    //    Es la "ficha" completa que transporta cada regla lógica (comparacion, cond_and, condicion).
    //    Separa los saltos pendientes en dos grupos según a dónde deben ir:
    typedef struct {
        ListaSaltos truelist;  // Lista de saltos que deben ir al bloque VERDADERO (entrar al si/mientras)
        ListaSaltos falselist; // Lista de saltos que deben ir al bloque FALSO (salir del si/mientras)
    } CondAttr;

    ListaSaltos crear_lista(int pos);
    ListaSaltos lista_vacia(void);
    ListaSaltos combinar_listas(ListaSaltos l1, ListaSaltos l2);
    void backpatch(ListaSaltos l, int destino);
    void apilar_cond(CondAttr c);
    CondAttr desapilar_cond(void);
}

%union {
    char str_val[50];  // Para variables y tercetos aritméticos (ej. "x", "[T0]")
    int int_val;       // Para el marcador M (guarda un número de terceto)
    CondAttr cond_val; // NUEVO: Para reglas lógicas (transporta truelist y falselist)
}

%token <str_val> ID LIT_RACIONAL LIT_CADENA

%type <str_val> expresion termino factor operador_comparacion valor_retorno
// Le asignamos el tipo cond_val (CondAttr) a todas las reglas lógicas:
%type <cond_val> condicion cond_or cond_and cond_factor comparacion
%type <int_val> M


/* Identificadores y Literales */
//%token ID LIT_RACIONAL LIT_CADENA

/* Palabras reservadas */
%token PR_DEF PR_PRINCIPAL PR_RACIONAL PR_SI PR_SINO PR_MIENTRAS PR_MOSTRAR PR_RETORNAR

/* Operadores y Asignación */
%token OP_ASIG OP_SUMA OP_RESTA OP_MULT OP_DIV

/* Comparadores y Operadores Lógicos */
%token COMP_MAYOR COMP_MENOR COMP_MAYOR_IGUAL COMP_MENOR_IGUAL COMP_IGUAL COMP_DISTINTO
%token OP_AND OP_OR

/* Delimitadores */
%token PAR_IZQ PAR_DER LLAVE_IZQ LLAVE_DER PUNTO_COMA COMA

/* Precedencia y Asociatividad de Operadores Lógicos y Relacionales */
%left OP_OR
%left OP_AND
%left COMP_IGUAL COMP_DISTINTO
%left COMP_MAYOR COMP_MENOR COMP_MAYOR_IGUAL COMP_MENOR_IGUAL

%%
/* ==========================================
   NIVEL 3: REGLAS GRAMATICALES (RAZIO)
   ========================================== */

//programa: declaraciones funciones PR_PRINCIPAL PAR_IZQ PAR_DER bloque;

// 15/11
programa: 
    declaraciones 
    {
        crear_terceto("IR_PRINCIPAL", "-", "-");
    }
    funciones 
    PR_PRINCIPAL PAR_IZQ PAR_DER 
    {
        strcpy(funcion_actual, "principal");
        crear_terceto("PRINCIPAL", "-", "-");
    }
    bloque
;





declaraciones: declaracion | /* vacio */ ;

declaracion: PR_RACIONAL lista_variables PUNTO_COMA;

lista_variables: variable | lista_variables COMA variable;

//variable: ID | ID OP_ASIG LIT_RACIONAL;
// 12/11
/*
variable:
      ID
    | ID OP_ASIG LIT_RACIONAL
      {
          crear_terceto("=", $1, $3);
      }
;*/


// 15/11
variable:
      ID
      {
          registrar_variable($1);
      }
    | ID OP_ASIG LIT_RACIONAL
      {
          registrar_variable($1);
          crear_terceto("=", $1, $3);
      }
    
;

funciones: /* vacio */ | funciones funcion;

//funcion: PR_DEF ID PAR_IZQ PAR_DER bloque;

// 15/11
funcion: 
    PR_DEF ID PAR_IZQ PAR_DER 
    {
        strcpy(funcion_actual, $2);
        crear_terceto("FUNC_INICIO", $2, "-");
    }
    bloque
    {
        crear_terceto("FUNC_FIN", $2, "-");
        strcpy(funcion_actual, "");
    }
;


bloque: LLAVE_IZQ sentencias LLAVE_DER;

sentencias: /* vacio */ | sentencias sentencia;

sentencia: asignacion | si | mientras | mostrar | retornar | llamada_funcion 
         | error PUNTO_COMA { yyerrok; }
         | error LLAVE_DER { yyerrok; };
//asignacion: ID OP_ASIG expresion PUNTO_COMA;

/*asignacion: 
    ID OP_ASIG expresion PUNTO_COMA
    {
        crear_terceto("=", $1, $3);
    }
;*/

//15/11
asignacion: 
    ID OP_ASIG expresion PUNTO_COMA
    {
        verificar_variable($1);
        crear_terceto("=", $1, $3);
    }
;



//expresion: termino | expresion OP_SUMA termino | expresion OP_RESTA termino;


expresion:
    termino
    {
        strcpy($$, $1);  // copia el valor semántico de termino en expresion
        				// esto permite transportar lexemas o referencias a tercetos
                         // entre reglas, utilizando el campo str_val[50] de %union.
    }
  | expresion OP_SUMA termino
    {
        int pos = crear_terceto("+", $1, $3);  // genera un terceto de suma y guarda su posición.
        sprintf($$, "[T%d]", pos);             // guarda la referencia al terceto como valor semántico de expresion.
    }
  | expresion OP_RESTA termino
    {
        int pos = crear_terceto("-", $1, $3);  // genera un terceto de resta y guarda su posición.
        sprintf($$, "[T%d]", pos);             // guarda la referencia al terceto como valor semántico de expresion.
    }
;




///termino: factor | termino OP_MULT factor | termino OP_DIV factor;

termino:
    factor
    {
        strcpy($$, $1);  // Copia el valor semántico de factor en termino.
    }
  | termino OP_MULT factor
    {
        int pos = crear_terceto("*", $1, $3);  // Genera un terceto de multiplicación y guarda su posición.
		sprintf($$, "[T%d]", pos); }// Guarda la referencia al terceto generado como valor semántico de termino EN LA PILA
								   //para que pueda utilizarse en operaciones posteriores.    
  | termino OP_DIV factor
    {
        int pos = crear_terceto("/", $1, $3);  // Genera un terceto de división y guarda su posición.
        sprintf($$, "[T%d]", pos);             // Guarda en $$ la referencia al terceto generado (ej: [T2]).
    }
;


factor: 
    LIT_RACIONAL { strcpy($$, $1); }
  | ID 
    { 
        verificar_variable($1);
        strcpy($$, $1); 
    }
  | PAR_IZQ expresion PAR_DER { strcpy($$, $2); }
  | ID PAR_IZQ PAR_DER
    { 
        if (strcmp($1, funcion_actual) == 0) {
            printf("\n[ERROR SEMANTICO - Linea %d]: La funcion '%s' no puede llamarse a si misma.\n", num_linea, $1);
            error_semantico = 1;
        }
        int pos = crear_terceto("CALL", $1, "-");
        sprintf($$, "[T%d]", pos);
    }
;
	


/* Marca auxiliar: captura el índice del próximo terceto a generarse */
M:
    /* vacio */
    {
        $$ = indice_terceto;
    }
;


////si: PR_SI PAR_IZQ condicion PAR_DER bloque | PR_SI PAR_IZQ condicion PAR_DER bloque PR_SINO bloque;


//4-11//--------------------------TERCETO MANEJO DE SALTOS----------------------------------------------------------------------------------------------------------------------

//--------------------------TERCETO MANEJO DE SALTOS (SI / SINO)----------------------------------------------------------------------------------------------------------------------


/* Regla auxiliar que emite el BF apenas termina de evaluar la condición */
marca_si:
    PR_SI PAR_IZQ condicion PAR_DER
    {
        /* Todos los saltos verdaderos entran directo al bloque del SI (indice_terceto) */
        backpatch($3.truelist, indice_terceto);
        /* Guardamos la falselist para parcharla cuando sepamos dónde termina el bloque */
        apilar_cond($3);
    }
;

si:
    marca_si bloque
    {
        /* SI SIMPLE: los saltos falsos salen al final del bloque */
        CondAttr c = desapilar_cond();
        backpatch(c.falselist, indice_terceto);
    }
  | marca_si bloque PR_SINO
    {
        /* TRANSICIÓN AL SINO */
        CondAttr c = desapilar_cond();
        int pos_bi = crear_terceto("BI", "?", "-");
        apilar(pos_bi);

        /* Los saltos falsos de la condición van al inicio del SINO */
        backpatch(c.falselist, indice_terceto);
    }
    bloque
    {
        /* FIN DEL SINO: el BI salta al final */
        int pos_bi = desapilar();
        char dir_salida[15];
        sprintf(dir_salida, "[T%d]", indice_terceto);
        modificar_terceto_op1(pos_bi, dir_salida);
    }
;
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//4-11//--------------------------TERCETO MANEJO DE SALTOS----------------------------------------------------------------------------------------------------------------------
















//mientras: PR_MIENTRAS PAR_IZQ condicion PAR_DER bloque;


//--------------------------TERCETO MANEJO DE SALTOS (MIENTRAS)----------------------------------------------------------------------------------------------------------------------


//-------------------------- TERCETOS: MANEJO DE SALTOS (MIENTRAS) --------------------------

/* Marca 1: Guarda la posición donde comienza la evaluación de la condición. */
inicio_mientras:
    PR_MIENTRAS
    {
        apilar(indice_terceto); /* Guarda el inicio de la condición para el BI final */
    }
;

condicion_mientras:
    inicio_mientras PAR_IZQ condicion PAR_DER
    {
        /* Todos los saltos verdaderos entran al cuerpo del bucle */
        backpatch($3.truelist, indice_terceto);
        /* Apilamos la falselist para salir del bucle al terminar */
        apilar_cond($3);
    }
;

mientras:
    condicion_mientras bloque
    {
        CondAttr c = desapilar_cond();
        int pos_inicio = desapilar();

        /* Salto incondicional al inicio de la condición */
        char dir_inicio[15];
        sprintf(dir_inicio, "[T%d]", pos_inicio);
        crear_terceto("BI", dir_inicio, "-");

        /* Los saltos falsos salen afuera del mientras */
        backpatch(c.falselist, indice_terceto);
    }
;

//-------------------------- FIN DEL MANEJO DE SALTOS (MIENTRAS) --------------------------

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//mostrar: PR_MOSTRAR PAR_IZQ argumentos_mostrar PAR_DER PUNTO_COMA;

//argumentos_mostrar: ID | LIT_CADENA | LIT_CADENA COMA lista_ids_mostrar;

//lista_ids_mostrar: ID | lista_ids_mostrar COMA ID;


//12/11
mostrar: PR_MOSTRAR PAR_IZQ argumentos_mostrar PAR_DER PUNTO_COMA;

argumentos_mostrar:
      ID
      {
          verificar_variable($1);
          crear_terceto("MOSTRAR_ID", $1, "-");
      }
    | LIT_CADENA
      {
          crear_terceto("MOSTRAR_STR", $1, "-");
      }
    | LIT_CADENA
      {
          crear_terceto("MOSTRAR_STR", $1, "-");
      }
      COMA lista_ids_mostrar
;

lista_ids_mostrar:
      ID
      {
          verificar_variable($1);
          crear_terceto("MOSTRAR_ID", $1, "-");
      }
    | lista_ids_mostrar COMA ID
      {
          verificar_variable($3);
          crear_terceto("MOSTRAR_ID", $3, "-");
      }
;

















retornar: 
    PR_RETORNAR PAR_IZQ valor_retorno PAR_DER PUNTO_COMA
    {
        crear_terceto("RETORNAR", $3, "-");
    }
;

valor_retorno: 
    ID 
    { 
        verificar_variable($1);
        strcpy($$, $1); 
    }
  | LIT_RACIONAL 
    { 
        strcpy($$, $1); 
    }
;

llamada_funcion: 
    ID PAR_IZQ PAR_DER PUNTO_COMA
    {
        if (strcmp($1, funcion_actual) == 0) {
            printf("\n[ERROR SEMANTICO - Linea %d]: La funcion '%s' no puede llamarse a si misma.\n", num_linea, $1);
            error_semantico = 1;
        }
        crear_terceto("CALL", $1, "-");
    }
;



/* Transmiten el valor semántico de la condición entre las reglas, sin generar tercetos. */
/*condicion: cond_or { strcpy($$, $1); } ;
cond_or: cond_and { strcpy($$, $1); } ;
cond_and: cond_factor { strcpy($$, $1); } ;
cond_factor:						/* Permite una comparación directa o una condición entre paréntesis. */
   /* comparacion { strcpy($$, $1); }
  | PAR_IZQ condicion PAR_DER { strcpy($$, $2); }
;	*/




//4-11//--------------------------TERCETO MANEJO DE AND $ OR----------------------------------------------------------------------------------------------------------------------

// Generación de tercetos lógicos AND y OR

/*-------------------------- MANEJO DE AND Y OR CON CORTOCIRCUITO --------------------------*/

condicion:
    cond_or
    {
        $$ = $1; // Propaga el paquete final (truelist y falselist) hacia 'si' o 'mientras'[cite: 3, 4]
    }
;

cond_or:
    cond_and
    {
        $$ = $1; // Caso base: sin 'or', pasa las listas intactas hacia arriba[cite: 3, 4]
    }
  | cond_or OP_OR
    {
        // Acción intermedia (justo tras leer 'or', antes de la 2da condición):
        // Toma el último salto BF del operando izquierdo ($1) y lo pasa a BV (salto por verdadero)[cite: 3, 4]
        int ultimo_salto = $1.falselist.list[$1.falselist.count - 1];
        strcpy(tabla_tercetos[ultimo_salto].operacion, "BV");

        // Mueve ese salto de falselist a truelist: si la 1ra condición da verdadera,
        // saltará directo al bloque verdadero (cortocircuito) sin evaluar la 2da[cite: 3, 4, 6]
        $1.truelist = combinar_listas($1.truelist, crear_lista(ultimo_salto));
        $1.falselist.count--;
    }
    M cond_and
    {
        // Si el 1er operando ($1) dio falso, completa sus saltos falsos apuntando a M ($4)
        // para que pase a evaluar el 2do operando ($5)[cite: 3, 4, 6]
        backpatch($1.falselist, $4);

        // Une los saltos verdaderos de ambos lados (basta que uno se cumpla para entrar)[cite: 3, 4, 6]
        $$.truelist  = combinar_listas($1.truelist, $5.truelist);

        // Si el 2do operando también falla, sale por los saltos falsos de $5[cite: 3, 4, 6]
        $$.falselist = $5.falselist;
    }
;

cond_and:
    cond_factor
    {
        // Caso base: copia las listas de la condición hacia arriba.
        $$ = $1;
    }
  | cond_and OP_AND M cond_factor
    {
        // Si la primera condición es verdadera, sus saltos pendientes
        // deben continuar desde el comienzo de la segunda condición (M).
        backpatch($1.truelist, $3);

        // El AND será verdadero solamente si la segunda condición es verdadera.
        // Por eso, conserva los saltos verdaderos de la segunda condición.
        $$.truelist = $4.truelist;

        // Si cualquiera de las dos condiciones es falsa, todo el AND es falso.
        // Por eso, junta los saltos falsos de ambas condiciones.
        $$.falselist = combinar_listas($1.falselist, $4.falselist);
    }
;

cond_factor:
    comparacion
    {
        // Caso base: si es una comparación simple (ej. x > 5/1),
        // simplemente sube su estructura CondAttr (truelist y falselist) hacia arriba sin cambios.
        $$ = $1;
    }
  | PAR_IZQ condicion PAR_DER
    {
        // Agrupación entre paréntesis (ej. (a > 1/1 or b < 2/1)):
        // descarta los paréntesis ($1 y $3) y propaga tal cual las listas truelist y falselist
        // de la condición interna ($2), conservando la prioridad que impusieron los paréntesis.
        $$ = $2;
    }
;

comparacion:
    expresion operador_comparacion expresion
    {
        // 1. Genera el terceto que realiza la comparación.
        //    $1 es el operando izquierdo, $2 es el operador y $3 es el derecho.
        //    Ejemplo: [T0] (>, x, 5/1)
        int pos_comp = crear_terceto($2, $1, $3);

        // 2. Declara una cadena para guardar la referencia al terceto de comparación.
        char ref_comp[15];

        // 3. Convierte el índice del terceto en una referencia textual.
        //    Ejemplo: si pos_comp = 0, guarda "[T0]" en ref_comp.
        sprintf(ref_comp, "[T%d]", pos_comp);

        // 4. Genera un salto por falso (BF) que utiliza la comparación anterior.
        //    Si la comparación es falsa, salta al destino indicado por "?".
        //    Ese destino se completará más adelante mediante backpatching.
        //    Ejemplo: [T1] (BF, [T0], ?)
        int pos_bf = crear_terceto("BF", ref_comp, "?");

        // 5. Inicializa vacía la lista de saltos por verdadero (truelist).
        //    Si la comparación es verdadera, no necesita saltar:
        //    continúa ejecutando el siguiente terceto.
        $$.truelist = lista_vacia();

        // 6. Crea la lista de saltos por falso (falselist).
        //    Guarda en ella el índice del BF generado (pos_bf),
        //    para completar su destino más adelante.
        $$.falselist = crear_lista(pos_bf);
    }
;

operador_comparacion: 							/* Convierte el token de comparación en su símbolo correspondiente (>, <, ==, etc.). */
    COMP_MAYOR        { strcpy($$, ">"); }
  | COMP_MENOR        { strcpy($$, "<"); }
  | COMP_MAYOR_IGUAL  { strcpy($$, ">="); }
  | COMP_MENOR_IGUAL  { strcpy($$, "<="); }
  | COMP_IGUAL        { strcpy($$, "=="); }
  | COMP_DISTINTO     { strcpy($$, "!="); }
;
//--------------------------TERCETO MANEJO DE SALTOS----------------------------------------------------------------------------------------------------------------------








   
%%

/* ==========================================
   NIVEL 4: CÓDIGO C ADICIONAL
   ========================================== */

// Crea una lista con UN solo índice de terceto (ej. crear_lista(1) -> list={1}, count=1)
ListaSaltos crear_lista(int pos) {
    ListaSaltos l;
    l.list[0] = pos; // Guarda el índice del terceto en la primera posición
    l.count = 1;     // Marca que la lista tiene 1 elemento
    return l;
}

// Crea una lista vacía (count = 0), cuando todavía no hay saltos pendientes
ListaSaltos lista_vacia(void) {
    ListaSaltos l;
    l.count = 0;
    return l;
}

// Une dos listas de saltos en una sola.
// CLAVE PARA EL 'AND': junta los saltos falsos de la 1ra condición con los de la 2da.
// Ejemplo: combinar_listas([1], [3]) -> devuelve [1, 3].
ListaSaltos combinar_listas(ListaSaltos l1, ListaSaltos l2) {
    ListaSaltos res = l1; // Copia la primera lista
    for (int i = 0; i < l2.count; i++) {
        if (res.count < 100) {
            res.list[res.count++] = l2.list[i]; // Le agrega al final cada elemento de la segunda
        }
    }
    return res;
}

// BACKPATCH ("parchear hacia atrás"):
// Recorre todos los índices guardados en la lista 'l' y les cambia el "?" por "[T<destino>]".
// Ejemplo: backpatch([1, 3], 6) -> entra a tabla_tercetos[1] y tabla_tercetos[3] y les pone "[T6]".
void backpatch(ListaSaltos l, int destino) {
    char dir[15];
    sprintf(dir, "[T%d]", destino); // Arma el texto "[T6]"
    for (int i = 0; i < l.count; i++) {
        modificar_terceto(l.list[i], dir); // Reemplaza el "?" en el terceto correspondiente
    }
}

// Pila para guardar CondAttr mientras se analiza el cuerpo { ... } de un 'si' o 'mientras'
CondAttr pila_condiciones[50];
int tope_cond = 0;

void apilar_cond(CondAttr c) {
    if (tope_cond < 50) {
        pila_condiciones[tope_cond++] = c; // Guarda la ficha en la cima de la pila
    }
}

CondAttr desapilar_cond(void) {
    if (tope_cond > 0) {
        return pila_condiciones[--tope_cond]; // Saca y devuelve la última ficha guardada
    }
    CondAttr vacia = { lista_vacia(), lista_vacia() };
    return vacia;
}


void yyerror(const char *mensaje) {
    printf("\n[ERROR SINTACTICO - Linea %d]: Cerca de '%s'\n", num_linea, buffer);
    error_sintactico = 1; /* <-- Marca que hubo error */
}

int main(int argc, char *argv[]) {
    if ((inputFile = fopen("ejemplo.raz", "r")) == NULL) {
        printf("Error al abrir el archivo 'ejemplo.raz'\n");
        return 1;
    }

    printf("==================================================================\n");
    printf("        COMPILADOR - ANALISIS SINTACTICO         \n");
    printf("==================================================================\n\n");

    yyparse();

    /* Valida que no hayan ocurrido errores en ninguna de las dos fases */
		if (error_sintactico == 0 && error_lexico == 0 && error_semantico == 0) {
        printf("\n\n[EXITO] El programa es sintacticamente correcto.\n");
        exportarTS();
        
        //TERCETO
        exportar_tercetos(); /* <-- Aca generas el archivo de tercetos */
        // 12/11
        generar_assembler();   /* <-- Genera salida.asm */
        
        
        
        /* Ensamblado y enlazado automático */
        printf("\n[PROCESO] Ensamblando con NASM...\n");
        if (system("nasm -f win64 salida.asm -o salida.obj") == 0) {
            printf("[PROCESO] Enlazando con GCC...\n");
            if (system("gcc salida.obj -o salida.exe") == 0) {
                printf("[EXITO] Ejecutable 'salida.exe' generado correctamente.\n");
            } else {
                printf("[ERROR] Fallo el enlazado con GCC.\n");
            }
        } else {
            printf("[ERROR] Fallo el ensamblado con NASM.\n");
        }
        
        
        
        
        
    } else {
        printf("\n\n[ERROR] Se encontraron errores durante el analisis.\n");
    }

    fclose(inputFile);
    return 0;
}
