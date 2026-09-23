#include <stdio.h>
#include <stdlib.h>
#include "tokens.h"
#include "lexico.h"

int main(int argc, char *argv[]) {
    const char *nombre_archivo = "ejemplo.raz";
    if (argc > 1) {
        nombre_archivo = argv[1];
    }

    FILE *arch = fopen(nombre_archivo, "r");
    if (!arch) {
        printf("Error: No se pudo abrir el archivo fuente '%s'.\n", nombre_archivo);
        return 1;
    }

    printf("=================================================================\n");
    printf("     INICIANDO ANALIZADOR LÉXICO - COMPILADOR RAZIO (GRUPO C)    \n");
    printf("=================================================================\n\n");

    inicializar_lexico(arch);

    int token;
    while ((token = yylex()) != 0) {
        printf("[Línea %2d] TOKEN: %-18s | Código: %d", num_linea, obtener_nombre_token(token), token);
        if (token == ID || token == LIT_CADENA) {
            printf(" | Lexema: \"%s\"", yylval.lexema);
        } else if (token == LIT_RACIONAL) {
            printf(" | Lexema: \"%s\" (num: %lld, den: %lld)", yylval.lexema, yylval.numerador, yylval.denominador);
        }
        printf("\n");
    }

    printf("\n=================================================================\n");
    printf("                PROCESAMIENTO LÉXICO FINALIZADO                  \n");
    printf("=================================================================\n");

    fclose(arch);
    return 0;
}
