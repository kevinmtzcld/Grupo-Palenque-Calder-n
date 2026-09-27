#ifndef LEXICO_H
#define LEXICO_H

#include <stdio.h>

// Atributo global expuesto hacia el Analizador Sintáctico / YACC
typedef struct {
    char lexema[256];
    long long numerador;   // Para LIT_RACIONAL
    long long denominador; // Para LIT_RACIONAL
    int pos_ts;            // Índice en la Tabla de Símbolos
} YYSTYPE;

extern YYSTYPE yylval;
extern int num_linea;

// Inicializa el léxico con el archivo fuente
void inicializar_lexico(FILE *archivo);

// Función principal solicitada por YACC/Sintáctico para pedir el siguiente token
int yylex(void);

// Convierte un código numérico de token a su nombre legible
const char* obtener_nombre_token(int token);

#endif // LEXICO_H
