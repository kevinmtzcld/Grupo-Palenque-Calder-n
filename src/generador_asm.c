#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "generador_asm.h"

/* ============================================================================
 * 1. ACCESO A LAS ESTRUCTURAS GLOBALES DEL LÉXICO Y SINTÁCTICO
 * ============================================================================ */
#define MAX_BUFFER 256

typedef struct {
    char nombre[MAX_BUFFER];
    char tipo[20];
    char valor[MAX_BUFFER];
    int longitud;
} RegTablaSimbolos;

extern RegTablaSimbolos tabla_simbolos[];
extern int cant_simbolos;

typedef struct {
    char operacion[15];
    char operando1[50];
    char operando2[50];
} Terceto;

extern Terceto tabla_tercetos[];
extern int indice_terceto;

/* ============================================================================
 * 2. FUNCIONES AUXILIARES PARA RESOLVER OPERANDOS Y ETIQUETAS
 * ============================================================================ */

/* Carga en reg_num el numerador y en reg_den el denominador de cualquier operando:
 * - Si es "[T2]" -> lee [rel T2_num] y [rel T2_den]
 * - Si es "3/4"  -> carga 3 en reg_num y 4 en reg_den
 * - Si es "5"    -> carga 5 en reg_num y 1 en reg_den
 * - Si es "a"    -> lee [rel a_num] y [rel a_den]
 */
void emitir_carga_operando(FILE *asm_file, const char *op, const char *reg_num, const char *reg_den) {
    if (op[0] == '[' && op[1] == 'T') {
        int idx = atoi(op + 2);
        fprintf(asm_file, "    mov %s, [rel T%d_num]\n", reg_num, idx);
        fprintf(asm_file, "    mov %s, [rel T%d_den]\n", reg_den, idx);
    } else if (isdigit((unsigned char)op[0]) || (op[0] == '-' && isdigit((unsigned char)op[1]))) {
        char copia[64];
        strcpy(copia, op);
        char *barra = strchr(copia, '/');
        if (barra != NULL) {
            *barra = '\0';
            long long num = atoll(copia);
            long long den = atoll(barra + 1);
            fprintf(asm_file, "    mov %s, %lld\n", reg_num, num);
            fprintf(asm_file, "    mov %s, %lld\n", reg_den, den);
        } else {
            long long num = atoll(copia);
            fprintf(asm_file, "    mov %s, %lld\n", reg_num, num);
            fprintf(asm_file, "    mov %s, 1\n", reg_den);
        }
    } else {
        fprintf(asm_file, "    mov %s, [rel %s_num]\n", reg_num, op);
        fprintf(asm_file, "    mov %s, [rel %s_den]\n", reg_den, op);
    }
}

/* Extrae el número N de una referencia "[TN]" para formar "ETIQ_N" */
int obtener_indice_salto(const char *ref) {
    if (ref[0] == '[' && ref[1] == 'T') {
        return atoi(ref + 2);
    }
    return -1;
}

/* Busca el índice STR_k en la Tabla de Símbolos dado el texto literal */
int buscar_indice_cadena(const char *texto) {
    int idx = 0;
    for (int i = 0; i < cant_simbolos; i++) {
        if (strcmp(tabla_simbolos[i].tipo, "LIT_CADENA") == 0) {
            if (strcmp(tabla_simbolos[i].valor, texto) == 0) {
                return idx;
            }
            idx++;
        }
    }
    return 0;
}

/* ============================================================================
 * 3. BIBLIOTECA DE RUTINAS AUXILIARES (MOTOR RACIONAL GRUPO C)
 * ============================================================================ */
void emitir_rutinas_auxiliares(FILE *asm_file) {
    fprintf(asm_file, "\n; ==========================================================\n");
    fprintf(asm_file, "; BIBLIOTECA DE RUTINAS AUXILIARES (MOTOR RACIONAL GRUPO C)\n");
    fprintf(asm_file, "; ==========================================================\n\n");

    /* RUTINA 1: normalizar_racional (EAX = num, EBX = den -> EAX/EBX simplificado) */
    fprintf(asm_file, "normalizar_racional:\n");
    fprintf(asm_file, "    cmp ebx, 0\n");
    fprintf(asm_file, "    je error_div_cero\n\n");

    fprintf(asm_file, "    cmp eax, 0\n");
    fprintf(asm_file, "    jne .revisar_signo\n");
    fprintf(asm_file, "    mov ebx, 1\n");
    fprintf(asm_file, "    ret\n\n");

    fprintf(asm_file, ".revisar_signo:\n");
    fprintf(asm_file, "    cmp ebx, 0\n");
    fprintf(asm_file, "    jg .calcular_mcd\n");
    fprintf(asm_file, "    neg eax\n");
    fprintf(asm_file, "    neg ebx\n\n");

    fprintf(asm_file, ".calcular_mcd:\n");
    fprintf(asm_file, "    mov ecx, eax\n");
    fprintf(asm_file, "    cmp ecx, 0\n");
    fprintf(asm_file, "    jge .mcd_abs_ok\n");
    fprintf(asm_file, "    neg ecx\n");
    fprintf(asm_file, ".mcd_abs_ok:\n");
    fprintf(asm_file, "    mov r8d, ebx\n\n");

    fprintf(asm_file, ".bucle_euclides:\n");
    fprintf(asm_file, "    cmp r8d, 0\n");
    fprintf(asm_file, "    je .mcd_listo\n");
    fprintf(asm_file, "    push rax\n");
    fprintf(asm_file, "    mov eax, ecx\n");
    fprintf(asm_file, "    cdq\n");
    fprintf(asm_file, "    idiv r8d\n");
    fprintf(asm_file, "    mov ecx, r8d\n");
    fprintf(asm_file, "    mov r8d, edx\n");
    fprintf(asm_file, "    pop rax\n");
    fprintf(asm_file, "    jmp .bucle_euclides\n\n");

    fprintf(asm_file, ".mcd_listo:\n");
    fprintf(asm_file, "    cdq\n");
    fprintf(asm_file, "    idiv ecx\n");
    fprintf(asm_file, "    mov r9d, eax\n");
    fprintf(asm_file, "    mov eax, ebx\n");
    fprintf(asm_file, "    cdq\n");
    fprintf(asm_file, "    idiv ecx\n");
    fprintf(asm_file, "    mov ebx, eax\n");
    fprintf(asm_file, "    mov eax, r9d\n");
    fprintf(asm_file, "    ret\n\n");

    /* RUTINA 2: mcd_64_positivo (RAX, RBX -> RAX = MCD(|RAX|, |RBX|)) */
    fprintf(asm_file, "mcd_64_positivo:\n");
    fprintf(asm_file, "    cmp rax, 0\n");
    fprintf(asm_file, "    jge .m64_a_ok\n");
    fprintf(asm_file, "    neg rax\n");
    fprintf(asm_file, ".m64_a_ok:\n");
    fprintf(asm_file, "    cmp rbx, 0\n");
    fprintf(asm_file, "    jge .m64_b_ok\n");
    fprintf(asm_file, "    neg rbx\n");
    fprintf(asm_file, ".m64_b_ok:\n");
    fprintf(asm_file, ".m64_loop:\n");
    fprintf(asm_file, "    cmp rbx, 0\n");
    fprintf(asm_file, "    je .m64_fin\n");
    fprintf(asm_file, "    cqo\n");
    fprintf(asm_file, "    idiv rbx\n");
    fprintf(asm_file, "    mov rax, rbx\n");
    fprintf(asm_file, "    mov rbx, rdx\n");
    fprintf(asm_file, "    jmp .m64_loop\n");
    fprintf(asm_file, ".m64_fin:\n");
    fprintf(asm_file, "    ret\n\n");

    /* RUTINA 3: verificar_overflow_64 (RAX = valor intermedio de 64 bits) */
    fprintf(asm_file, "verificar_overflow_64:\n");
    fprintf(asm_file, "    mov r10, 2147483647\n");
    fprintf(asm_file, "    cmp rax, r10\n");
    fprintf(asm_file, "    jg error_overflow\n");
    fprintf(asm_file, "    mov r10, -2147483648\n");
    fprintf(asm_file, "    cmp rax, r10\n");
    fprintf(asm_file, "    jl error_overflow\n");
    fprintf(asm_file, "    ret\n\n");

    /* RUTINA 4: imprimir_racional (ECX = num, EDX = den) */
    fprintf(asm_file, "imprimir_racional:\n");
    fprintf(asm_file, "    sub rsp, 40\n");
    fprintf(asm_file, "    mov r8d, edx\n");
    fprintf(asm_file, "    mov edx, ecx\n");
    fprintf(asm_file, "    lea rcx, [rel fmt_rac]\n");
    fprintf(asm_file, "    call printf\n");
    fprintf(asm_file, "    add rsp, 40\n");
    fprintf(asm_file, "    ret\n\n");

    /* RUTINAS 5 y 6: Manejadores de error en ejecución */
    fprintf(asm_file, "error_div_cero:\n");
    fprintf(asm_file, "    sub rsp, 40\n");
    fprintf(asm_file, "    lea rcx, [rel err_cero]\n");
    fprintf(asm_file, "    call printf\n");
    fprintf(asm_file, "    mov ecx, 1\n");
    fprintf(asm_file, "    call exit\n\n");

    fprintf(asm_file, "error_overflow:\n");
    fprintf(asm_file, "    sub rsp, 40\n");
    fprintf(asm_file, "    lea rcx, [rel err_ovf]\n");
    fprintf(asm_file, "    call printf\n");
    fprintf(asm_file, "    mov ecx, 1\n");
    fprintf(asm_file, "    call exit\n");
}

/* ============================================================================
 * 4. TRADUCCIÓN DE TERCETOS A ASSEMBLER x86-64
 * ============================================================================ */
void emitir_tercetos_asm(FILE *asm_file) {
    for (int i = 0; i < indice_terceto; i++) {
        char *op  = tabla_tercetos[i].operacion;
        char *op1 = tabla_tercetos[i].operando1;
        char *op2 = tabla_tercetos[i].operando2;

        fprintf(asm_file, "ETIQ_%d: ; [%s, %s, %s]\n", i, op, op1, op2);

        /* 1. ASIGNACIÓN: (=, destino, origen) */
        if (strcmp(op, "=") == 0) {
            emitir_carga_operando(asm_file, op2, "eax", "ebx");
            fprintf(asm_file, "    call normalizar_racional\n");
            fprintf(asm_file, "    mov [rel %s_num], eax\n", op1);
            fprintf(asm_file, "    mov [rel %s_den], ebx\n\n", op1);
        }
        /* 2. SUMA Y RESTA: (+, a/b, c/d) o (-, a/b, c/d) */
        else if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0) {
            emitir_carga_operando(asm_file, op1, "eax", "ebx");
            fprintf(asm_file, "    movsxd r12, eax\n");
            fprintf(asm_file, "    movsxd r13, ebx\n");
            emitir_carga_operando(asm_file, op2, "eax", "ebx");
            fprintf(asm_file, "    movsxd r14, eax\n");
            fprintf(asm_file, "    movsxd r15, ebx\n");

            /* Numerador = num1 * den2 (+/-) num2 * den1 (en 64 bits con control previo de overflow) */
            fprintf(asm_file, "    mov rax, r12\n");
            fprintf(asm_file, "    imul rax, r15\n");
            fprintf(asm_file, "    call verificar_overflow_64\n");
            fprintf(asm_file, "    mov rcx, rax\n");

            fprintf(asm_file, "    mov rax, r14\n");
            fprintf(asm_file, "    imul rax, r13\n");
            fprintf(asm_file, "    call verificar_overflow_64\n");

            if (strcmp(op, "+") == 0) {
                fprintf(asm_file, "    add rcx, rax\n");
            } else {
                fprintf(asm_file, "    sub rcx, rax\n");
            }
            fprintf(asm_file, "    mov rax, rcx\n");
            fprintf(asm_file, "    call verificar_overflow_64\n");
            fprintf(asm_file, "    push rax\n");

            /* Denominador = den1 * den2 */
            fprintf(asm_file, "    mov rax, r13\n");
            fprintf(asm_file, "    imul rax, r15\n");
            fprintf(asm_file, "    call verificar_overflow_64\n");
            fprintf(asm_file, "    mov ebx, eax\n");
            fprintf(asm_file, "    pop rax\n");

            fprintf(asm_file, "    call normalizar_racional\n");
            fprintf(asm_file, "    mov [rel T%d_num], eax\n", i);
            fprintf(asm_file, "    mov [rel T%d_den], ebx\n\n", i);
        }
        /* 3. MULTIPLICACIÓN Y DIVISIÓN: (*, a/b, c/d) o (/, a/b, c/d) con simplificación cruzada */
        else if (strcmp(op, "*") == 0 || strcmp(op, "/") == 0) {
            emitir_carga_operando(asm_file, op1, "eax", "ebx");
            fprintf(asm_file, "    movsxd r12, eax\n");
            fprintf(asm_file, "    movsxd r13, ebx\n");
            emitir_carga_operando(asm_file, op2, "eax", "ebx");

            if (strcmp(op, "/") == 0) {
                /* División por cero antes de invertir la segunda fracción */
                fprintf(asm_file, "    cmp eax, 0\n");
                fprintf(asm_file, "    je error_div_cero\n");
                /* Dividir por c/d equivale a multiplicar por d/c */
                fprintf(asm_file, "    movsxd r14, ebx\n");
                fprintf(asm_file, "    movsxd r15, eax\n");
            } else {
                fprintf(asm_file, "    movsxd r14, eax\n");
                fprintf(asm_file, "    movsxd r15, ebx\n");
            }

            /* Simplificación cruzada 1: g1 = MCD(|num1|, |den2|) */
            fprintf(asm_file, "    mov rax, r12\n");
            fprintf(asm_file, "    mov rbx, r15\n");
            fprintf(asm_file, "    call mcd_64_positivo\n");
            fprintf(asm_file, "    mov rcx, rax\n");
            fprintf(asm_file, "    mov rax, r12\n");
            fprintf(asm_file, "    cqo\n");
            fprintf(asm_file, "    idiv rcx\n");
            fprintf(asm_file, "    mov r12, rax\n");
            fprintf(asm_file, "    mov rax, r15\n");
            fprintf(asm_file, "    cqo\n");
            fprintf(asm_file, "    idiv rcx\n");
            fprintf(asm_file, "    mov r15, rax\n");

            /* Simplificación cruzada 2: g2 = MCD(|num2|, |den1|) */
            fprintf(asm_file, "    mov rax, r14\n");
            fprintf(asm_file, "    mov rbx, r13\n");
            fprintf(asm_file, "    call mcd_64_positivo\n");
            fprintf(asm_file, "    mov rcx, rax\n");
            fprintf(asm_file, "    mov rax, r14\n");
            fprintf(asm_file, "    cqo\n");
            fprintf(asm_file, "    idiv rcx\n");
            fprintf(asm_file, "    mov r14, rax\n");
            fprintf(asm_file, "    mov rax, r13\n");
            fprintf(asm_file, "    cqo\n");
            fprintf(asm_file, "    idiv rcx\n");
            fprintf(asm_file, "    mov r13, rax\n");

            /* Producto final con control de overflow en 64 bits */
            fprintf(asm_file, "    mov rax, r12\n");
            fprintf(asm_file, "    imul rax, r14\n");
            fprintf(asm_file, "    call verificar_overflow_64\n");
            fprintf(asm_file, "    push rax\n");

            fprintf(asm_file, "    mov rax, r13\n");
            fprintf(asm_file, "    imul rax, r15\n");
            fprintf(asm_file, "    call verificar_overflow_64\n");
            fprintf(asm_file, "    mov ebx, eax\n");
            fprintf(asm_file, "    pop rax\n");

            fprintf(asm_file, "    call normalizar_racional\n");
            fprintf(asm_file, "    mov [rel T%d_num], eax\n", i);
            fprintf(asm_file, "    mov [rel T%d_den], ebx\n\n", i);
        }
        /* 4. COMPARADORES RELACIONALES: (>, <, >=, <=, ==, !=) */
        else if (strcmp(op, ">") == 0 || strcmp(op, "<") == 0 ||
                 strcmp(op, ">=") == 0 || strcmp(op, "<=") == 0 ||
                 strcmp(op, "==") == 0 || strcmp(op, "!=") == 0) {
            emitir_carga_operando(asm_file, op1, "eax", "ebx");
            fprintf(asm_file, "    movsxd r12, eax\n");
            fprintf(asm_file, "    movsxd r13, ebx\n");
            emitir_carga_operando(asm_file, op2, "eax", "ebx");
            fprintf(asm_file, "    movsxd r14, eax\n");
            fprintf(asm_file, "    movsxd r15, ebx\n");

            /* Producto cruzado exacto en 64 bits: (num1 * den2) vs (num2 * den1) */
            fprintf(asm_file, "    mov rax, r12\n");
            fprintf(asm_file, "    imul rax, r15\n");
            fprintf(asm_file, "    mov rcx, r14\n");
            fprintf(asm_file, "    imul rcx, r13\n");
            fprintf(asm_file, "    cmp rax, rcx\n");

            const char *set_inst = "sete";
            if (strcmp(op, ">") == 0)       set_inst = "setg";
            else if (strcmp(op, "<") == 0)  set_inst = "setl";
            else if (strcmp(op, ">=") == 0) set_inst = "setge";
            else if (strcmp(op, "<=") == 0) set_inst = "setle";
            else if (strcmp(op, "==") == 0) set_inst = "sete";
            else if (strcmp(op, "!=") == 0) set_inst = "setne";

            fprintf(asm_file, "    %s al\n", set_inst);
            fprintf(asm_file, "    movzx eax, al\n");
            fprintf(asm_file, "    mov [rel T%d_num], eax\n", i);
            fprintf(asm_file, "    mov dword [rel T%d_den], 1\n\n", i);
        }
        /* 5. OPERADORES LÓGICOS: (AND, op1, op2) y (OR, op1, op2) */
        else if (strcmp(op, "AND") == 0 || strcmp(op, "OR") == 0) {
            emitir_carga_operando(asm_file, op1, "eax", "ebx");
            emitir_carga_operando(asm_file, op2, "ecx", "edx");
            fprintf(asm_file, "    cmp eax, 0\n");
            fprintf(asm_file, "    setne al\n");
            fprintf(asm_file, "    cmp ecx, 0\n");
            fprintf(asm_file, "    setne cl\n");
            if (strcmp(op, "AND") == 0) {
                fprintf(asm_file, "    and al, cl\n");
            } else {
                fprintf(asm_file, "    or al, cl\n");
            }
            fprintf(asm_file, "    movzx eax, al\n");
            fprintf(asm_file, "    mov [rel T%d_num], eax\n", i);
            fprintf(asm_file, "    mov dword [rel T%d_den], 1\n\n", i);
        }
        /* 6. SALTO CONDICIONAL POR FALSO: (BF, condicion, [Td]) */
        else if (strcmp(op, "BF") == 0) {
            int dest = obtener_indice_salto(op2);
            emitir_carga_operando(asm_file, op1, "eax", "ebx");
            fprintf(asm_file, "    cmp eax, 0\n");
            fprintf(asm_file, "    je ETIQ_%d\n\n", dest);
        }
        /* 7. SALTO INCONDICIONAL: (BI, [Td], -) */
        else if (strcmp(op, "BI") == 0) {
            int dest = obtener_indice_salto(op1);
            fprintf(asm_file, "    jmp ETIQ_%d\n\n", dest);
        }
        /* 8. SALIDA POR CONSOLA: MOSTRAR_ID y MOSTRAR_STR */
        else if (strcmp(op, "MOSTRAR_ID") == 0) {
            fprintf(asm_file, "    mov ecx, [rel %s_num]\n", op1);
            fprintf(asm_file, "    mov edx, [rel %s_den]\n", op1);
            fprintf(asm_file, "    call imprimir_racional\n\n");
        }
        else if (strcmp(op, "MOSTRAR_STR") == 0) {
            int idx_str = buscar_indice_cadena(op1);
            fprintf(asm_file, "    lea rcx, [rel fmt_str]\n");
            fprintf(asm_file, "    lea rdx, [rel STR_%d]\n", idx_str);
            fprintf(asm_file, "    call printf\n\n");
        }
        /* 9. SALTO A PRINCIPAL Y DEFINICIÓN/LLAMADA DE FUNCIONES */
        else if (strcmp(op, "IR_PRINCIPAL") == 0) {
            fprintf(asm_file, "    jmp ETIQ_PRINCIPAL\n\n");
        }
        else if (strcmp(op, "PRINCIPAL") == 0) {
            fprintf(asm_file, "ETIQ_PRINCIPAL:\n\n");
        }
        else if (strcmp(op, "FUNC_INICIO") == 0) {
            fprintf(asm_file, "FUNC_%s:\n", op1);
            fprintf(asm_file, "    sub rsp, 40\n\n");
        }
        else if (strcmp(op, "RETORNAR") == 0) {
            emitir_carga_operando(asm_file, op1, "eax", "ebx");
            fprintf(asm_file, "    call normalizar_racional\n");
            fprintf(asm_file, "    add rsp, 40\n");
            fprintf(asm_file, "    ret\n\n");
        }
        else if (strcmp(op, "FUNC_FIN") == 0) {
            fprintf(asm_file, "    mov eax, 0\n");
            fprintf(asm_file, "    mov ebx, 1\n");
            fprintf(asm_file, "    add rsp, 40\n");
            fprintf(asm_file, "    ret\n\n");
        }
        else if (strcmp(op, "CALL") == 0) {
            fprintf(asm_file, "    call FUNC_%s\n", op1);
            fprintf(asm_file, "    mov [rel T%d_num], eax\n", i);
            fprintf(asm_file, "    mov [rel T%d_den], ebx\n\n", i);
        }
    }

    /* Etiqueta final para saltos que apuntan al fin del programa */
    fprintf(asm_file, "ETIQ_%d: ; Fin de tercetos\n", indice_terceto);
}

/* ============================================================================
 * 5. FUNCIÓN PRINCIPAL DE GENERACIÓN
 * ============================================================================ */
void generar_assembler(void) {
    FILE *asm_file = fopen("salida.asm", "w");
    if (!asm_file) {
        printf("[ERROR] No se pudo crear el archivo 'salida.asm'\n");
        return;
    }

    fprintf(asm_file, "; ==========================================================\n");
    fprintf(asm_file, "; CODIGO ASSEMBLER GENERADO POR COMPILADOR RAZIO (GRUPO C)\n");
    fprintf(asm_file, "; Arquitectura: x86-64 (NASM -f win64)\n");
    fprintf(asm_file, "; ==========================================================\n\n");

    fprintf(asm_file, "global main\n");
    fprintf(asm_file, "extern printf\n");
    fprintf(asm_file, "extern exit\n\n");

    /* Sección .data */
    fprintf(asm_file, "section .data\n");
    fprintf(asm_file, "    fmt_rac  db \"%%d/%%d\", 10, 0\n");
    fprintf(asm_file, "    fmt_str  db \"%%s\", 10, 0\n");
    fprintf(asm_file, "    err_ovf  db \"ERROR EN EJECUCION: Overflow en operacion racional\", 10, 0\n");
    fprintf(asm_file, "    err_cero db \"ERROR EN EJECUCION: Division por cero\", 10, 0\n\n");

    fprintf(asm_file, "    ; --- Variables Racionales ---\n");
    for (int i = 0; i < cant_simbolos; i++) {
        if (strcmp(tabla_simbolos[i].tipo, "ID") == 0) {
            fprintf(asm_file, "    %s_num dd 0\n", tabla_simbolos[i].nombre);
            fprintf(asm_file, "    %s_den dd 1\n", tabla_simbolos[i].nombre);
        }
    }

    fprintf(asm_file, "\n    ; --- Literales de Cadena ---\n");
    int idx_cadena = 0;
    for (int i = 0; i < cant_simbolos; i++) {
        if (strcmp(tabla_simbolos[i].tipo, "LIT_CADENA") == 0) {
            fprintf(asm_file, "    STR_%d db \"%s\", 0\n", idx_cadena++, tabla_simbolos[i].valor);
        }
    }

    fprintf(asm_file, "\n    ; --- Temporales de Tercetos ---\n");
    for (int i = 0; i < indice_terceto; i++) {
        fprintf(asm_file, "    T%d_num dd 0\n", i);
        fprintf(asm_file, "    T%d_den dd 1\n", i);
    }

    /* Sección .text */
    fprintf(asm_file, "\nsection .text\n");
    fprintf(asm_file, "main:\n");
    fprintf(asm_file, "    sub rsp, 40\n\n");

    /* Traducción completa de todos los tercetos */
    emitir_tercetos_asm(asm_file);

    fprintf(asm_file, "\n    xor ecx, ecx\n");
    fprintf(asm_file, "    call exit\n");

    /* Biblioteca de rutinas del motor racional */
    emitir_rutinas_auxiliares(asm_file);

    fclose(asm_file);
    printf("[EXITO] Archivo 'salida.asm' generado correctamente.\n");
}
