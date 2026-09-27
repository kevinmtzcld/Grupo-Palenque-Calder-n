#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_BUFFER 256
#define MAX_ID_LEN 20         /* D6: LONGITUD MÁXIMA DE IDENTIFICADOR EN 20 CARACTERES */
#define MAX_INT_32 2147483647L /* D2: LÍMITE SUPERIOR DE 32 BITS CON SIGNO */

/* 1. TABLA DE TOKENS (SECCIÓN 4 DE PARALEXICO.TXT) */
#define PR_DEF          257
#define PR_PRINCIPAL    258
#define PR_RACIONAL     259
#define PR_SI           260
#define PR_SINO         261
#define PR_MIENTRAS     262
#define PR_MOSTRAR      263
#define PR_RETORNAR     264
#define ID              265
#define LIT_RACIONAL    266
#define LIT_CADENA      267
#define OP_ASIG         268
#define OP_SUMA         269
#define OP_RESTA        270
#define OP_MULT         271
#define OP_DIV          272
#define COMP_MAYOR      273
#define COMP_MENOR      274
#define COMP_MAYOR_IGUAL 275
#define COMP_MENOR_IGUAL 276
#define COMP_IGUAL      277
#define COMP_DISTINTO   278
#define OP_AND          279
#define OP_OR           280
#define PAR_IZQ         281
#define PAR_DER         282
#define LLAVE_IZQ       283
#define LLAVE_DER       284
#define PUNTO_COMA      285
#define COMA            286

/* TABLA DE SÍMBOLOS */
typedef struct {
    char nombre[MAX_BUFFER];
    char tipo[20];
    char valor[MAX_BUFFER];
    int longitud;
} RegTablaSimbolos;

RegTablaSimbolos tabla_simbolos[500];
int cant_simbolos = 0;

/* VARIABLES GLOBALES */
int num_linea = 1;
int error_lexico = 0;
FILE *inputFile = NULL;
char buffer[MAX_BUFFER];
int buf_idx = 0;
char c_actual;
int ultimo_token_devuelto = 0;
long val_num_actual = 0;

/* ALFABETO */
typedef enum {
    COL_LETRA = 0, COL_DIGITO, COL_MAS, COL_MENOS, COL_POR, COL_BARRA,
    COL_IGUAL, COL_MENOR, COL_MAYOR, COL_ADMIRACION, COL_PAR_IZQ,
    COL_PAR_DER, COL_LLAVE_IZQ, COL_LLAVE_DER, COL_PUNTO_COMA,
    COL_DOS_PUNTOS, COL_COMA, COL_COMILLA, COL_BLANCO, COL_OTRO,
    NUM_COLUMNAS
} Evento;

typedef void (*AccionSemantica)(void);

/* TRADUCCIÓN DE CÓDIGO DE TOKEN A SU NOMBRE */
const char* obtener_nombre_token(int codigo) {
    switch (codigo) {
        case PR_DEF: return "PR_DEF";
        case PR_PRINCIPAL: return "PR_PRINCIPAL";
        case PR_RACIONAL: return "PR_RACIONAL";
        case PR_SI: return "PR_SI";
        case PR_SINO: return "PR_SINO";
        case PR_MIENTRAS: return "PR_MIENTRAS";
        case PR_MOSTRAR: return "PR_MOSTRAR";
        case PR_RETORNAR: return "PR_RETORNAR";
        case ID: return "ID";
        case LIT_RACIONAL: return "LIT_RACIONAL";
        case LIT_CADENA: return "LIT_CADENA";
        case OP_ASIG: return "OP_ASIG";
        case OP_SUMA: return "OP_SUMA";
        case OP_RESTA: return "OP_RESTA";
        case OP_MULT: return "OP_MULT";
        case OP_DIV: return "OP_DIV";
        case COMP_MAYOR: return "COMP_MAYOR";
        case COMP_MENOR: return "COMP_MENOR";
        case COMP_MAYOR_IGUAL: return "COMP_MAYOR_IGUAL";
        case COMP_MENOR_IGUAL: return "COMP_MENOR_IGUAL";
        case COMP_IGUAL: return "COMP_IGUAL";
        case COMP_DISTINTO: return "COMP_DISTINTO";
        case OP_AND: return "OP_AND";
        case OP_OR: return "OP_OR";
        case PAR_IZQ: return "PAR_IZQ";
        case PAR_DER: return "PAR_DER";
        case LLAVE_IZQ: return "LLAVE_IZQ";
        case LLAVE_DER: return "LLAVE_DER";
        case PUNTO_COMA: return "PUNTO_COMA";
        case COMA: return "COMA";
        default: return "TOKEN_DESCONOCIDO";
    }
}

void insertar_ts(const char *nombre, const char *tipo, const char *valor, int longitud) {
    for (int i = 0; i < cant_simbolos; i++) {
        if (strcmp(tabla_simbolos[i].nombre, nombre) == 0) return;
    }
    if (cant_simbolos < 500) {
        strcpy(tabla_simbolos[cant_simbolos].nombre, nombre);
        strcpy(tabla_simbolos[cant_simbolos].tipo, tipo);
        strcpy(tabla_simbolos[cant_simbolos].valor, valor);
        tabla_simbolos[cant_simbolos].longitud = longitud;
        cant_simbolos++;
    }
}

/* ACCIONES SEMÁNTICAS */
void f1(void) { 
    buf_idx = 0; 
    buffer[buf_idx++] = c_actual; 
    buffer[buf_idx] = '\0'; 
}

void f2(void) { 
    buf_idx = 0; 
    val_num_actual = c_actual - '0';
    buffer[buf_idx++] = c_actual; 
    buffer[buf_idx] = '\0'; 
}

void f3a(void) { 
    if (buf_idx < MAX_ID_LEN) { 
        buffer[buf_idx++] = c_actual; 
        buffer[buf_idx] = '\0'; 
    } else if (buf_idx == MAX_ID_LEN) { 
        printf("\n[ADVERTENCIA - Línea %d]: ID '%s...' excede los %d caracteres y fue truncado.\n", num_linea, buffer, MAX_ID_LEN); 
        buf_idx++; 
    }
}

void f3b(void) { 
    if (isdigit(c_actual)) {
        val_num_actual = val_num_actual * 10 + (c_actual - '0');
        if (val_num_actual > MAX_INT_32) {
            printf("\n[ERROR LÉXICO - Línea %d]: Overflow en constante '%s%c' (Excede 32 bits).\n", num_linea, buffer, c_actual);
            error_lexico = 1;
        }
    } else if (c_actual == '/') {
        val_num_actual = 0;
    }

    if (buf_idx < MAX_BUFFER - 1) { 
        buffer[buf_idx++] = c_actual; 
        buffer[buf_idx] = '\0'; 
    }
}

void f4(void) {
    if (strcmp(buffer, "def") == 0) ultimo_token_devuelto = PR_DEF;
    else if (strcmp(buffer, "principal") == 0) ultimo_token_devuelto = PR_PRINCIPAL;
    else if (strcmp(buffer, "racional") == 0) ultimo_token_devuelto = PR_RACIONAL;
    else if (strcmp(buffer, "si") == 0) ultimo_token_devuelto = PR_SI;
    else if (strcmp(buffer, "sino") == 0) ultimo_token_devuelto = PR_SINO;
    else if (strcmp(buffer, "mientras") == 0) ultimo_token_devuelto = PR_MIENTRAS;
    else if (strcmp(buffer, "mostrar") == 0) ultimo_token_devuelto = PR_MOSTRAR;
    else if (strcmp(buffer, "retornar") == 0) ultimo_token_devuelto = PR_RETORNAR;
    else if (strcmp(buffer, "and") == 0) ultimo_token_devuelto = OP_AND;
    else if (strcmp(buffer, "or") == 0) ultimo_token_devuelto = OP_OR;
    else {
        if (strlen(buffer) > MAX_ID_LEN) {
            buffer[MAX_ID_LEN] = '\0';
        }
        ultimo_token_devuelto = ID;
        insertar_ts(buffer, "ID", "-", strlen(buffer));
    }
}

void f5(void) {
    char *barra = strchr(buffer, '/');
    if (barra != NULL && strcmp(barra, "/0") == 0) {
        printf("\n[ERROR LÉXICO/SEMÁNTICO - Línea %d]: División por cero en literal '%s'\n", num_linea, buffer);
        error_lexico = 1;
    }
    
    ultimo_token_devuelto = LIT_RACIONAL;
    
    if (barra == NULL) {
        char val_norm[MAX_BUFFER];
        sprintf(val_norm, "%s/1", buffer);
        insertar_ts(buffer, "LIT_RACIONAL", val_norm, strlen(buffer));
    } else {
        insertar_ts(buffer, "LIT_RACIONAL", buffer, strlen(buffer));
    }
}

void f7a(void) { 
    buf_idx = 0; 
    buffer[buf_idx] = '\0'; 
}

void f7b(void) { 
    if (buf_idx < MAX_BUFFER - 1) { 
        buffer[buf_idx++] = c_actual; 
        buffer[buf_idx] = '\0'; 
    }
}

void f7c(void) {
    ultimo_token_devuelto = LIT_CADENA;
    insertar_ts(buffer, "LIT_CADENA", buffer, strlen(buffer));
}

void f8(void) {
    /* Descarte de comentarios */
}

void fn(void) { 
    if (buf_idx == 0 && c_actual != ' ' && c_actual != '\t' && c_actual != '\r' && c_actual != '\n') {
        buffer[buf_idx++] = c_actual;
        buffer[buf_idx] = '\0';
    }
}

void fe(void) { 
    printf("\n[ERROR LÉXICO - Línea %d]: Carácter no permitido '%c'\n", num_linea, c_actual); 
    error_lexico = 1; 
}

/* MATRIZ DE FUNCIONES DE ACCIONES SEMÁNTICAS */
const AccionSemantica proceso[28][NUM_COLUMNAS] = {
/* e 0 */ { f1,  f2,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fe,  fn,  f7a, fn,  fe },
/* e 1 */ { f3a, f3a, f4,  f4,  f4,  f4,  f4,  f4,  f4,  f4,  f4,  f4,  f4,  f4,  f4,  f4,  f4,  f4,  f4,  fe },
/* e 2 */ { fe,  fe,  fe,  fe,  fe,  fe,  fn,  fe,  fe,  fe,  fe,  fe,  fe,  fe,  fe,  fe,  fe,  fe,  fe,  fe },
/* e 3 */ { fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fe },
/* e 4 */ { f5,  f3b, f5,  f5,  f5,  f3b, f5,  f5,  f5,  f5,  f5,  f5,  f5,  f5,  f5,  f5,  f5,  f5,  f5,  fe },
/* e 5 */ { fe,  f3b, fe,  fe,  fe,  fe,  fe,  fe,  fe,  fe,  fe,  fe,  fe,  fe,  fe,  fe,  fe,  fe,  fe,  fe },
/* e 6 */ { f5,  f3b, f5,  f5,  f5,  f5,  f5,  f5,  f5,  f5,  f5,  f5,  f5,  f5,  f5,  f5,  f5,  f5,  f5,  fe },
/* e 7 */ { fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fe },
/* e 8 */ { fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fe },
/* e 9 */ { fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fe },
/* e10 */ { fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fe },
/* e11 */ { fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fe },
/* e12 */ { fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fe },
/* e13 */ { f7b, f7b, f7b, f7b, f7b, f7b, f7b, f7b, f7b, f7b, f7b, f7b, f7b, f7b, f7b, f7b, f7b, f7c, f7b, fe },
/* e14 */ { fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fe },
/* e15 */ { fn,  fn,  fn,  fn,  f8,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fe },
/* e16 */ { f8,  f8,  f8,  f8,  f8,  f8,  f8,  f8,  f8,  f8,  f8,  f8,  f8,  f8,  f8,  f8,  f8,  f8,  f8,  fe },
/* e17 */ { f8,  f8,  f8,  f8,  f8,  f8,  f8,  f8,  f8,  f8,  f8,  f8,  f8,  f8,  f8,  f8,  f8,  f8,  f8,  fe },
/* e18 */ { fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fe },
/* e19 */ { fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fe },
/* e20 */ { fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fe },
/* e21 */ { fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fe },
/* e22 */ { fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fe },
/* e23 */ { fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fe },
/* e24 */ { fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fe },
/* e25 */ { fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fe },
/* e26 */ { fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fe },
/* e27 */ { fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fn,  fe }
};

/* MATRIZ DE TRANSICIÓN DE NUEVOS ESTADOS */
const int nuevo_estado[28][NUM_COLUMNAS] = {
/*e0 */ { 1,  4, 25, 26, 27, 15, 11,  9,  7,  2, 21, 22, 23, 24, 18, -1, 19, 13,  0, -1 },
/*e1 */ { 1,  1, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -1 },
/*e2 */ {-1, -1, -1, -1, -1, -1,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
/*e3 */ {-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -1 },
/*e4 */ {-2,  4, -2, -2, -2,  5, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -1 },
/*e5 */ {-1,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
/*e6 */ {-2,  6, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -1 },
/*e7 */ {-2, -2, -2, -2, -2, -2,  8, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -1 },
/*e8 */ {-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -1 },
/*e9 */ {-2, -2, -2, -2, -2, -2, 10, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -1 },
/*e10*/ {-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -1 },
/*e11*/ {-2, -2, -2, -2, -2, -2, 12, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -1 },
/*e12*/ {-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -1 },
/*e13*/ {13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 13, 13 },
/*e14*/ {-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -1 },
/*e15*/ {-2, -2, -2, -2, 16, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -1 },
/*e16*/ {16, 16, 16, 16, 17, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16 },
/*e17*/ {16, 16, 16, 16, 16,  0, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16 },
/*e18*/ {-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -1 },
/*e19*/ {-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -1 },
/*e20*/ { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/*e21*/ {-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -1 },
/*e22*/ {-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -1 },
/*e23*/ {-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -1 },
/*e24*/ {-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -1 },
/*e25*/ {-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -1 },
/*e26*/ {-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -1 },
/*e27*/ {-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -1 }
};

static const int token_por_estado[28] = {
    0, 265, 0, 278, 266, 0, 266, 273, 275, 274, 276,
    268, 277, 0, 267, 272, 0, 0, 285, 286, 0, 
    281, 282, 283, 284, 269, 270, 271
};

Evento get_evento(int c) {
    if (isalpha(c)) return COL_LETRA;
    if (isdigit(c)) return COL_DIGITO;
    switch (c) {
        case '+': return COL_MAS;
        case '-': return COL_MENOS;
        case '*': return COL_POR;
        case '/': return COL_BARRA;
        case '=': return COL_IGUAL;
        case '<': return COL_MENOR;
        case '>': return COL_MAYOR;
        case '!': return COL_ADMIRACION;
        case '(': return COL_PAR_IZQ;
        case ')': return COL_PAR_DER;
        case '{': return COL_LLAVE_IZQ;
        case '}': return COL_LLAVE_DER;
        case ';': return COL_PUNTO_COMA;
        case ':': return COL_DOS_PUNTOS;
        case ',': return COL_COMA;
        case '"': return COL_COMILLA;
        case ' ': case '\t': case '\r': case '\n': return COL_BLANCO;
        default: return COL_OTRO;
    }
}

int yylex(void) {
    int estado = 0;
    int columna;
    int c;

    ultimo_token_devuelto = 0;
    buf_idx = 0;
    buffer[0] = '\0';

    while ((c = fgetc(inputFile)) != EOF) {
        c_actual = (char)c;
        if (c == '\n') num_linea++;

        columna = get_evento(c);

        (*proceso[estado][columna])();

        int est_sig = nuevo_estado[estado][columna];

        if (est_sig == -2) {
            ungetc(c, inputFile);
            if (c == '\n') num_linea--;

            if (ultimo_token_devuelto != 0) {
                return ultimo_token_devuelto;
            }
            return token_por_estado[estado];

        } else if (est_sig == -1) {
            estado = 0;
            buf_idx = 0;
            buffer[0] = '\0';
        } else {
            estado = est_sig;
        }
    }
    return 0;
}

void exportarTS(void) {
    FILE *f_ts = fopen("tabla_simbolos.txt", "w");
    
    printf("\n=================================================================\n");
    printf("                  TABLA DE SÍMBOLOS GENERADA                     \n");
    printf("=================================================================\n");
    printf("%-20s | %-15s | %-20s | %-8s\n", "NOMBRE", "TIPO", "VALOR", "LONGITUD");
    printf("-----------------------------------------------------------------\n");

    if (f_ts != NULL) {
        fprintf(f_ts, "NOMBRE,TIPO,VALOR,LONGITUD\n");
    }

    for (int i = 0; i < cant_simbolos; i++) {
        printf("%-20s | %-15s | %-20s | %-8d\n", 
               tabla_simbolos[i].nombre, 
               tabla_simbolos[i].tipo, 
               tabla_simbolos[i].valor, 
               tabla_simbolos[i].longitud);

        if (f_ts != NULL) {
            fprintf(f_ts, "%s,%s,%s,%d\n", 
                    tabla_simbolos[i].nombre, 
                    tabla_simbolos[i].tipo, 
                    tabla_simbolos[i].valor, 
                    tabla_simbolos[i].longitud);
        }
    }

    if (f_ts != NULL) {
        fclose(f_ts);
        printf("\n[TS]: Exportada exitosamente al archivo 'tabla_simbolos.txt'\n");
    }
}

int main(int argc, char *argv[]) {
    if ((inputFile = fopen("ejemplo.raz", "r")) == NULL) {
        printf("Error al abrir el archivo 'ejemplo.raz'\n");
        return 1;
    }

    printf("=================================================================================\n");
    printf("           REPORTE DE ANALIZADOR LÉXICO - COMPILADOR RAZIO (GRUPO C)            \n");
    printf("=================================================================================\n");
    printf("%-8s | %-18s | %-8s | %-30s\n", "LÍNEA", "TOKEN", "CÓDIGO", "LEXEMA");
    printf("---------------------------------------------------------------------------------\n");

    int token;
    while (!feof(inputFile)) {
        token = yylex();
        if (token != 0) {
            printf("[Línea %2d] | %-18s | %-8d | '%s'\n", 
                   num_linea, 
                   obtener_nombre_token(token), 
                   token, 
                   buffer);
        }
    }

    fclose(inputFile);

    if (error_lexico == 0) {
        printf("\n - Compilacion AL EXITOSA - \n");
        exportarTS();
    } else {
        printf("\n - Analisis Lexico completo con ERRORES - \n");
    }

    return 0;
}
