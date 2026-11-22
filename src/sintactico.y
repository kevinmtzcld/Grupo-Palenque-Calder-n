%{
/* ==========================================
   NIVEL 1: DECLARACIONES EN C
   ========================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// 12/11
#include "generador_asm.h"

#include <stdlib.h>

/* Elementos externos que vienen del módulo léxico y entorno C */
extern int yylex(void);
extern int num_linea;
extern int error_lexico;
extern char buffer[];
extern FILE *inputFile;
extern void exportarTS(void);

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
/* 1. Define que los elementos de Bison transportarán texto (char[50]) en vez de enteros , CONSERVAR LEXEMA PARA luego crear el terceto*/
%union {
    char str_val[50];
}
/* 2. Le asigna ese contenedor de texto a los tokens que traen nombres de variables o valores */
%token <str_val> ID LIT_RACIONAL LIT_CADENA
/* 3. Permite que estas reglas devuelvan texto ($$) para propagar variables o tercetos ([T_n]) */
%type <str_val> expresion termino factor condicion cond_or cond_and cond_factor comparacion operador_comparacion marca_si inicio_mientras condicion_mientras valor_retorno
//------------------------------------------------------------------------------------------------------------------------------------------------



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
	
	





////si: PR_SI PAR_IZQ condicion PAR_DER bloque | PR_SI PAR_IZQ condicion PAR_DER bloque PR_SINO bloque;


//4-11//--------------------------TERCETO MANEJO DE SALTOS----------------------------------------------------------------------------------------------------------------------

//--------------------------TERCETO MANEJO DE SALTOS (SI / SINO)----------------------------------------------------------------------------------------------------------------------


/* Regla auxiliar que emite el BF apenas termina de evaluar la condición */
marca_si: 
    PR_SI PAR_IZQ condicion PAR_DER
    {
        int pos_bf = crear_terceto("BF", $3, "?");
        apilar(pos_bf);
    }
;

si: 
    marca_si bloque
    {
        /* SI SIMPLE (Sin SINO) */
        int pos_bf = desapilar();		//termina el bloque y ya se sabe el indice al que debe saltar el bf pendiente
        char dir_salida[10];
        sprintf(dir_salida, "[T%d]", indice_terceto);
        
        modificar_terceto(pos_bf, dir_salida);
    }
  | marca_si bloque PR_SINO
    {
        /* TRANSICIÓN AL SINO */
        int pos_bf = desapilar();	//DESAPILA LA POSICION DEL BF PORQUE AHORA LA VA COMPLETAR
									//Y  PORQUE AHORA VA A APILAR LA POS DEL TERCETO BI QUE ES EL PROXIMO A COMPLETAR
        
        /* 1. Crea el BI con destino temporal en operando1 */
        int pos_bi = crear_terceto("BI", "?", "-");
        apilar(pos_bi);
        
        /* 2. Parchea el BF para que apunte al terceto pos_bi + 1 (inicio del sino) */
        char dir_sino[10];
        sprintf(dir_sino, "[T%d]", indice_terceto); // copia en dir_sino el indice al terceto inicio del sino
        modificar_terceto(pos_bf, dir_sino);		//COMPLETA EL LA DIRECCIÓN DE SALTO DEL TERCETO BF PENDIENTE, SALTA DIRECTO AL BLOQUE SINO
    }
    bloque
    {
        /* FIN DEL SINO: Parchea el BI con modificar_terceto_op1 */
        int pos_bi = desapilar();	//RECUPERA LA POS DEL TERCETO DE BI A COMPLETAR
        char dir_salida[10];	
        sprintf(dir_salida, "[T%d]", indice_terceto); //COPIA EL INDICE DEL PROX TERCETO A "DIR_SALIDA" INDICE_TERCETO YA TIENE SIEMPRE EL 
        											//INDICE DEL PROX TERCETO
        
        modificar_terceto_op1(pos_bi, dir_salida);//COMPLETA EL LA DIRECCIÓN DE SALTO DEL TERCETO BI (POS_BI) PENDIENTE
													//SALTA DIRECTO AL PRIMER TERCETO FUERA DEL SINO (DIR_SALIDA)
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
        apilar(indice_terceto); // Guarda la posición de retorno para reevaluar la condición al finalizar cada iteración.
    }
;

/* Marca 2: Genera el salto por falso (BF) después de evaluar la condición. */
condicion_mientras:
    inicio_mientras PAR_IZQ condicion PAR_DER
    {
        int pos_bf = crear_terceto("BF", $3, "?"); // Genera el salto de salida con destino pendiente.
        apilar(pos_bf); // Guarda la posición del BF para completar su destino al finalizar el bucle.
    }
;

mientras:
    condicion_mientras bloque
    {
        int pos_bf = desapilar();     // Recupera la posición del BF para completar el salto de salida.
        int pos_inicio = desapilar(); // Recupera la posición donde debe volver a evaluarse la condición.

        /* Genera el salto incondicional (BI) para volver al inicio del bucle. */
        char dir_inicio[10];
        sprintf(dir_inicio, "[T%d]", pos_inicio); // Construye la referencia al terceto donde comienza la condición.
        crear_terceto("BI", dir_inicio, "-");     // Genera el salto de retorno al inicio del mientras.

        /* Completa el destino del BF para salir del bucle cuando la condición sea falsa. */
        char dir_salida[10];
        sprintf(dir_salida, "[T%d]", indice_terceto); // Obtiene la posición del próximo terceto, fuera del bucle.
        modificar_terceto(pos_bf, dir_salida);       // Reemplaza el destino pendiente del BF por la dirección de salida.
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

condicion:
    cond_or
    {
        strcpy($$, $1); // Transmite el resultado de toda la condición.
    }
;

cond_or:
    cond_and
    {
        strcpy($$, $1); // Si no hay OR, transmite el resultado de cond_and.
    }
  | cond_or OP_OR cond_and
    {
        int pos = crear_terceto("OR", $1, $3); // Genera un terceto OR combinando las dos condiciones.
        sprintf($$, "[T%d]", pos); // Guarda la referencia al terceto generado.
    }
;

cond_and:
    cond_factor
    {
        strcpy($$, $1); // Si no hay AND, transmite el resultado de cond_factor.
    }
  | cond_and OP_AND cond_factor
    {
        int pos = crear_terceto("AND", $1, $3); // Genera un terceto AND combinando las dos condiciones.
        sprintf($$, "[T%d]", pos); // Guarda la referencia al terceto generado.
    }
;

cond_factor:
    comparacion
    {
        strcpy($$, $1); // Transmite la referencia al terceto de la comparación.
    }
  | PAR_IZQ condicion PAR_DER
    {
        strcpy($$, $2); // Transmite el resultado de la condición entre paréntesis.
    }
;

//4-11//--------------------------TERCETO MANEJO DE AND $ OR----------------------------------------------------------------------------------------------------------------------



comparacion: expresion operador_comparacion expresion
{
        int pos = crear_terceto($2, $1, $3);//CREA TERCETO PARA UNA COMPARACIÓN NORMAL
        sprintf($$, "[T%d]", pos);
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
