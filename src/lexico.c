#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tokens.h"
#include "lexico.h"

#define MAX_BUFFER 256
#define MAX_ID_LEN 20

YYSTYPE yylval;
int num_linea = 1;
static FILE *fuente = NULL;
static char buffer[MAX_BUFFER];
static int buf_idx = 0;

typedef enum {
    COL_LETRA = 0, COL_DIGITO, COL_MAS, COL_MENOS, COL_POR, COL_BARRA,
    COL_IGUAL, COL_MENOR, COL_MAYOR, COL_ADMIRACION, COL_PAR_IZQ,
    COL_PAR_DER, COL_LLAVE_IZQ, COL_LLAVE_DER, COL_PUNTO_COMA,
    COL_DOS_PUNTOS, COL_COMA, COL_COMILLA, COL_BLANCO, COL_OTRO,
    NUM_COLUMNAS
} Evento;

static Evento obtener_evento(int c) {
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

static const int matriz_estados[28][NUM_COLUMNAS] = {
/*        L   D   +   -   *   /   =   <   >   !   (   )   {   }   ;   :   ,   "  BL  OTRO */
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

static void f1(char c) { buf_idx = 0; buffer[buf_idx++] = c; buffer[buf_idx] = '\0'; }
static void f2(char c) { buf_idx = 0; buffer[buf_idx++] = c; buffer[buf_idx] = '\0'; }
static void f3a(char c) {
    if (buf_idx < MAX_ID_LEN) { buffer[buf_idx++] = c; buffer[buf_idx] = '\0'; }
    else if (buf_idx == MAX_ID_LEN) { printf("[ADVERTENCIA - Línea %d]: Truncado ID '%s'\n", num_linea, buffer); buf_idx++; }
}
static void f3b(char c) { if (buf_idx < MAX_BUFFER - 1) { buffer[buf_idx++] = c; buffer[buf_idx] = '\0'; } }
static int f4(void) {
    if (strcmp(buffer, "def") == 0) return PR_DEF;
    if (strcmp(buffer, "principal") == 0) return PR_PRINCIPAL;
    if (strcmp(buffer, "racional") == 0) return PR_RACIONAL;
    if (strcmp(buffer, "si") == 0) return PR_SI;
    if (strcmp(buffer, "sino") == 0) return PR_SINO;
    if (strcmp(buffer, "mientras") == 0) return PR_MIENTRAS;
    if (strcmp(buffer, "mostrar") == 0) return PR_MOSTRAR;
    if (strcmp(buffer, "retornar") == 0) return PR_RETORNAR;
    if (strcmp(buffer, "and") == 0) return OP_AND;
    if (strcmp(buffer, "or") == 0) return OP_OR;
    strcpy(yylval.lexema, buffer);
    return ID;
}
static int f5(void) {
    strcpy(yylval.lexema, buffer);
    char *barra = strchr(buffer, '/');
    if (barra != NULL) { *barra = '\0'; yylval.numerador = atoll(buffer); yylval.denominador = atoll(barra + 1); }
    else { yylval.numerador = atoll(buffer); yylval.denominador = 1; }
    if (yylval.denominador == 0) printf("[ERROR LÉXICO - Línea %d]: Div por cero en '%s'.\n", num_linea, yylval.lexema);
    return LIT_RACIONAL;
}
static void f7a(void) { buf_idx = 0; buffer[buf_idx] = '\0'; }
static void f7b(char c) { if (buf_idx < MAX_BUFFER - 1) { buffer[buf_idx++] = c; buffer[buf_idx] = '\0'; } }
static int f7c(void) { strcpy(yylval.lexema, buffer); return LIT_CADENA; }
static void f8(void) {}
static void fn(void) {}
static void fe(char c) { printf("[ERROR LÉXICO - Línea %d]: Carácter no permitido '%c'\n", num_linea, c); }

void inicializar_lexico(FILE *arch) { fuente = arch; num_linea = 1; }

int yylex(void) {
    if (!fuente) return 0;
    int estado_actual = 0;
    int c;

    while ((c = fgetc(fuente)) != EOF) {
        if (c == '\n') num_linea++;
        Evento evento = obtener_evento(c);
        int estado_siguiente = matriz_estados[estado_actual][evento];

        switch (estado_actual) {
            case 0:
                if (evento == COL_LETRA) f1(c);
                else if (evento == COL_DIGITO) f2(c);
                else if (evento == COL_COMILLA) f7a();
                else {
                    buffer[0] = (char)c;
                    buffer[1] = '\0';
                }
                break;
            case 1: if (evento == COL_LETRA || evento == COL_DIGITO) f3a(c); break;
            case 4: if (evento == COL_DIGITO || evento == COL_BARRA) f3b(c); break;
            case 5: if (evento == COL_DIGITO) f3b(c); else fe(c); break;
            case 6: if (evento == COL_DIGITO) f3b(c); break;
            case 13: if (evento == COL_COMILLA) f7c(); else f7b(c); break;
            case 16: case 17: f8(); break;
            default: fn(); break;
        }

        if (estado_siguiente == -2) { // ST_FINAL_UNREAD
            ungetc(c, fuente);
            if (c == '\n') num_linea--;
            int tok = token_por_estado[estado_actual];
            if (estado_actual == 1) tok = f4();
            else if (estado_actual == 4 || estado_actual == 6) tok = f5();
            else if (estado_actual == 14) tok = f7c();
            else { strcpy(yylval.lexema, buffer); }
            return tok;
        } else if (estado_siguiente == -1) { // ST_ERROR
            fe(c);
            estado_actual = 0;
        } else {
            estado_actual = estado_siguiente;
        }
    }
    return 0;
}

const char* obtener_nombre_token(int token) {
    switch(token) {
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
        default: return "EOF";
    }
}
