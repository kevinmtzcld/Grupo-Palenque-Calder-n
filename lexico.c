

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tokens.h"
#include "lexico.h"

#define MAX_BUFFER 256
#define MAX_ID_LEN 20   // Decisión D6: Longitud máx de identificador
#define INT32_MIN_VAL -2147483648LL
#define INT32_MAX_VAL 2147483647LL

// Variables globales del Léxico
YYSTYPE yylval;
int num_linea = 1;
static FILE *fuente = NULL;
static char buffer[MAX_BUFFER];
static int buf_idx = 0;
static int overflow_flag = 0;

// Definición de columnas/eventos
typedef enum {
    COL_LETRA = 0,
    COL_DIGITO,
    COL_MAS,
    COL_MENOS,
    COL_POR,
    COL_BARRA,
    COL_IGUAL,
    COL_MENOR,
    COL_MAYOR,
    COL_ADMIRACION,
    COL_PAR_IZQ,
    COL_PAR_DER,
    COL_LLAVE_IZQ,
    COL_LLAVE_DER,
    COL_PUNTO_COMA,
    COL_DOS_PUNTOS,
    COL_COMA,
    COL_COMILLA,
    COL_BLANCO,
    COL_OTRO,
    NUM_COLUMNAS
} Evento;

// Nombres de Estados
typedef enum {
    E0 = 0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10,
    E11, E12, E13, E14, E15, E16, E17, E18, E19,
    E21, E22, E23, E24, E25, E26, E27,
    ST_ERROR, ST_FINAL_UNREAD, ST_FINAL_NO_UNREAD,
    NUM_ESTADOS
} Estado;

// 1. Clasificación del carácter a columna de evento
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
        case ' ':
        case '\t':
        case '\r':
        case '\n': return COL_BLANCO;
        default:   return COL_OTRO;
    }
}

// 2. Matriz de Transición de Nuevos Estados
static const int matriz_estados[27][NUM_COLUMNAS] = {
/*        L      D      +      -      *      /      =      <      >      !      (      )      {      }      ;      :      ,      "     BLANCO OTRO */
/*e0 */ { E1,   E4,   E25,   E26,   E27,   E15,   E11,   E9,    E7,    E2,    E21,   E22,   E23,   E24,   E18, ST_ERROR, E19,  E13,   E0,  ST_ERROR },
/*e1 */ { E1,   E1,  ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_ERROR },
/*e2 */ { ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR, E3, ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR },
/*e3 */ { ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_ERROR },
/*e4 */ { ST_FINAL_UNREAD, E4, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, E5, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_ERROR },
/*e5 */ { ST_ERROR, E6, ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR, ST_ERROR },
/*e6 */ { ST_FINAL_UNREAD, E6, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_ERROR },
/*e7 */ { ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, E8, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_ERROR },
/*e8 */ { ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_ERROR },
/*e9 */ { ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, E10, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_ERROR },
/*e10*/ { ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_ERROR },
/*e11*/ { ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, E12, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_ERROR },
/*e12*/ { ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_ERROR },
/*e13*/ { E13,  E13,   E13,   E13,   E13,   E13,   E13,   E13,   E13,   E13,   E13,   E13,   E13,   E13,   E13,   E13,   E13,   E14,   E13,  ST_ERROR },
/*e14*/ { ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_ERROR },
/*e15*/ { ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, E16, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_ERROR },
/*e16*/ { E16,  E16,   E16,   E16,   E17,   E16,   E16,   E16,   E16,   E16,   E16,   E16,   E16,   E16,   E16,   E16,   E16,   E16,   E16,  E16 },
/*e17*/ { E16,  E16,   E16,   E16,   E16,   E0,    E16,   E16,   E16,   E16,   E16,   E16,   E16,   E16,   E16,   E16,   E16,   E16,   E16,  E16 },
/*e18*/ { ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_ERROR },
/*e19*/ { ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_ERROR },
/*e21*/ { ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_ERROR },
/*e22*/ { ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_ERROR },
/*e23*/ { ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_ERROR },
/*e24*/ { ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_ERROR },
/*e25*/ { ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_ERROR },
/*e26*/ { ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_ERROR },
/*e27*/ { ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_FINAL_UNREAD, ST_ERROR }
};

// 3. Matriz de Mapeo de Tokens por Estado Final
static const int token_por_estado[28] = {
    0, 265, 0, 278, 266, 0, 266, 273, 275, 274, 276,
    268, 277, 0, 267, 272, 0, 0, 285, 286, 0, 281, 282, 283, 284, 269, 270, 271
};

// Acciones Semánticas (f1 - f8, fn, fe)
static void f1(char c) {
    buf_idx = 0;
    buffer[buf_idx++] = c;
    buffer[buf_idx] = '\0';
}

static void f2(char c) {
    buf_idx = 0;
    overflow_flag = 0;
    buffer[buf_idx++] = c;
    buffer[buf_idx] = '\0';
}

static void f3a(char c) {
    if (buf_idx < MAX_ID_LEN) {
        buffer[buf_idx++] = c;
        buffer[buf_idx] = '\0';
    } else if (buf_idx == MAX_ID_LEN) {
        printf("[ADVERTENCIA LÉXICA - Línea %d]: Identificador que excede %d caracteres fue truncado.\n", num_linea, MAX_ID_LEN);
        buf_idx++;
    }
}

static void f3b(char c) {
    if (buf_idx < MAX_BUFFER - 1) {
        buffer[buf_idx++] = c;
        buffer[buf_idx] = '\0';
    }
}

static int f4(void) {
    // Palabras Reservadas
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
    if (barra != NULL) {
        *barra = '\0';
        yylval.numerador = atoll(buffer);
        yylval.denominador = atoll(barra + 1);
    } else {
        yylval.numerador = atoll(buffer);
        yylval.denominador = 1;
    }

    // Regla D9: División por cero en literales
    if (yylval.denominador == 0) {
        printf("[ERROR LÉXICO - Línea %d]: División por cero detectada en el literal racional '%s'.\n", num_linea, yylval.lexema);
    }
    // Regla D2: Rango de 32 bits con signo
    if (yylval.numerador < INT32_MIN_VAL || yylval.numerador > INT32_MAX_VAL ||
        yylval.denominador < INT32_MIN_VAL || yylval.denominador > INT32_MAX_VAL) {
        printf("[ERROR LÉXICO - Línea %d]: Overflow en literal racional '%s' (excede 32 bits D2).\n", num_linea, yylval.lexema);
    }

    return LIT_RACIONAL;
}

static void f7a(void) {
    buf_idx = 0;
    buffer[buf_idx] = '\0';
}

static void f7b(char c) {
    if (buf_idx < MAX_BUFFER - 1) {
        buffer[buf_idx++] = c;
        buffer[buf_idx] = '\0';
    }
}

static int f7c(void) {
    strcpy(yylval.lexema, buffer);
    return LIT_CADENA;
}

static void f8(void) {
    // Acción para comentarios: ignorar contenido acumulado
}

static void fn(void) {
    // Función Nula
}

static void fe(char c) {
    printf("[ERROR LÉXICO - Línea %d]: Carácter no reconocido o secuencia inválida '%c' (ASCII %d).\n", num_linea, c, c);
}

void inicializar_lexico(FILE *arch) {
    fuente = arch;
    num_linea = 1;
}

// Implementación del bucle principal del autómata yylex()
int yylex(void) {
    if (!fuente) return 0;

    int estado_actual = E0;
    int estado_anterior = E0;
    int c;

    while ((c = fgetc(fuente)) != EOF) {
        // Conteo de líneas
        if (c == '\n') num_linea++;

        Evento evento = obtener_evento(c);
        int estado_siguiente = matriz_estados[estado_actual][evento];

        // Ejecución de Funciones Semánticas según transición
        switch (estado_actual) {
            case E0:
                if (evento == COL_LETRA) f1(c);
                else if (evento == COL_DIGITO) f2(c);
                else if (evento == COL_COMILLA) f7a();
                else fn();
                break;

            case E1:
                if (evento == COL_LETRA || evento == COL_DIGITO) f3a(c);
                break;

            case E4:
                if (evento == COL_DIGITO || evento == COL_BARRA) f3b(c);
                break;

            case E5:
                if (evento == COL_DIGITO) f3b(c);
                else fe(c);
                break;

            case E6:
                if (evento == COL_DIGITO) f3b(c);
                break;

            case E13:
                if (evento == COL_COMILLA) f7c();
                else f7b(c);
                break;

            case E16:
            case E17:
                f8();
                break;

            default:
                fn();
                break;
        }

        // Control de Estados Finales
        if (estado_siguiente == ST_FINAL_UNREAD) {
            ungetc(c, fuente); // Retracción (Unread)
            if (c == '\n') num_linea--; // Deshacer conteo si devolvemos salto de línea

            int tok = token_por_estado[estado_actual];
            if (estado_actual == E1) tok = f4();
            else if (estado_actual == E4 || estado_actual == E6) tok = f5();
            else if (estado_actual == E14) tok = f7c();
            else {
                strcpy(yylval.lexema, "");
                yylval.lexema[0] = (char)c;
                yylval.lexema[1] = '\0';
            }
            return tok;

        } else if (estado_siguiente == ST_ERROR) {
            fe(c);
            estado_actual = E0; // Recuperación de error simple
        } else {
            estado_anterior = estado_actual;
            estado_actual = estado_siguiente;
        }
    }

    return 0; // EOF
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
        default: return "EOF / DESCONOCIDO";
    }
}

