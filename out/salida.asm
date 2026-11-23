; ==========================================================
; CODIGO ASSEMBLER GENERADO POR COMPILADOR RAZIO (GRUPO C)
; Arquitectura: x86-64 (NASM -f win64)
; ==========================================================

global main
extern printf
extern exit

section .data
    fmt_rac  db "%d/%d", 10, 0
    fmt_str  db "%s", 10, 0
    err_ovf  db "ERROR EN EJECUCION: Overflow en operacion racional", 10, 0
    err_cero db "ERROR EN EJECUCION: Division por cero", 10, 0

    ; --- Variables Racionales ---
    notauno_num dd 0
    notauno_den dd 1
    notados_num dd 0
    notados_den dd 1
    promedio_num dd 0
    promedio_den dd 1
    asistencia_num dd 0
    asistencia_den dd 1
    calcularpromedio_num dd 0
    calcularpromedio_den dd 1

    ; --- Literales de Cadena ---
    STR_0 db "Promedio:", 0
    STR_1 db "El alumno aprueba", 0
    STR_2 db "El alumno desaprueba", 0

    ; --- Temporales de Tercetos ---
    T0_num dd 0
    T0_den dd 1
    T1_num dd 0
    T1_den dd 1
    T2_num dd 0
    T2_den dd 1
    T3_num dd 0
    T3_den dd 1
    T4_num dd 0
    T4_den dd 1
    T5_num dd 0
    T5_den dd 1
    T6_num dd 0
    T6_den dd 1
    T7_num dd 0
    T7_den dd 1
    T8_num dd 0
    T8_den dd 1
    T9_num dd 0
    T9_den dd 1
    T10_num dd 0
    T10_den dd 1
    T11_num dd 0
    T11_den dd 1
    T12_num dd 0
    T12_den dd 1
    T13_num dd 0
    T13_den dd 1
    T14_num dd 0
    T14_den dd 1
    T15_num dd 0
    T15_den dd 1
    T16_num dd 0
    T16_den dd 1
    T17_num dd 0
    T17_den dd 1
    T18_num dd 0
    T18_den dd 1
    T19_num dd 0
    T19_den dd 1
    T20_num dd 0
    T20_den dd 1
    T21_num dd 0
    T21_den dd 1

section .text
main:
    sub rsp, 40

ETIQ_0: ; [=, notauno, 6/1]
    mov eax, 6
    mov ebx, 1
    call normalizar_racional
    mov [rel notauno_num], eax
    mov [rel notauno_den], ebx

ETIQ_1: ; [=, notados, 8/1]
    mov eax, 8
    mov ebx, 1
    call normalizar_racional
    mov [rel notados_num], eax
    mov [rel notados_den], ebx

ETIQ_2: ; [=, asistencia, 4/5]
    mov eax, 4
    mov ebx, 5
    call normalizar_racional
    mov [rel asistencia_num], eax
    mov [rel asistencia_den], ebx

ETIQ_3: ; [IR_PRINCIPAL, -, -]
    jmp ETIQ_PRINCIPAL

ETIQ_4: ; [FUNC_INICIO, calcularpromedio, -]
FUNC_calcularpromedio:
    sub rsp, 40

ETIQ_5: ; [+, notauno, notados]
    mov eax, [rel notauno_num]
    mov ebx, [rel notauno_den]
    movsxd r12, eax
    movsxd r13, ebx
    mov eax, [rel notados_num]
    mov ebx, [rel notados_den]
    movsxd r14, eax
    movsxd r15, ebx
    mov rax, r12
    imul rax, r15
    call verificar_overflow_64
    mov rcx, rax
    mov rax, r14
    imul rax, r13
    call verificar_overflow_64
    add rcx, rax
    mov rax, rcx
    call verificar_overflow_64
    push rax
    mov rax, r13
    imul rax, r15
    call verificar_overflow_64
    mov ebx, eax
    pop rax
    call normalizar_racional
    mov [rel T5_num], eax
    mov [rel T5_den], ebx

ETIQ_6: ; [/, [T5], 2/1]
    mov eax, [rel T5_num]
    mov ebx, [rel T5_den]
    movsxd r12, eax
    movsxd r13, ebx
    mov eax, 2
    mov ebx, 1
    cmp eax, 0
    je error_div_cero
    movsxd r14, ebx
    movsxd r15, eax
    mov rax, r12
    mov rbx, r15
    call mcd_64_positivo
    mov rcx, rax
    mov rax, r12
    cqo
    idiv rcx
    mov r12, rax
    mov rax, r15
    cqo
    idiv rcx
    mov r15, rax
    mov rax, r14
    mov rbx, r13
    call mcd_64_positivo
    mov rcx, rax
    mov rax, r14
    cqo
    idiv rcx
    mov r14, rax
    mov rax, r13
    cqo
    idiv rcx
    mov r13, rax
    mov rax, r12
    imul rax, r14
    call verificar_overflow_64
    push rax
    mov rax, r13
    imul rax, r15
    call verificar_overflow_64
    mov ebx, eax
    pop rax
    call normalizar_racional
    mov [rel T6_num], eax
    mov [rel T6_den], ebx

ETIQ_7: ; [=, promedio, [T6]]
    mov eax, [rel T6_num]
    mov ebx, [rel T6_den]
    call normalizar_racional
    mov [rel promedio_num], eax
    mov [rel promedio_den], ebx

ETIQ_8: ; [RETORNAR, promedio, -]
    mov eax, [rel promedio_num]
    mov ebx, [rel promedio_den]
    call normalizar_racional
    add rsp, 40
    ret

ETIQ_9: ; [FUNC_FIN, calcularpromedio, -]
    mov eax, 0
    mov ebx, 1
    add rsp, 40
    ret

ETIQ_10: ; [PRINCIPAL, -, -]
ETIQ_PRINCIPAL:

ETIQ_11: ; [CALL, calcularpromedio, -]
    call FUNC_calcularpromedio
    mov [rel T11_num], eax
    mov [rel T11_den], ebx

ETIQ_12: ; [=, promedio, [T11]]
    mov eax, [rel T11_num]
    mov ebx, [rel T11_den]
    call normalizar_racional
    mov [rel promedio_num], eax
    mov [rel promedio_den], ebx

ETIQ_13: ; [MOSTRAR_STR, Promedio:, -]
    lea rcx, [rel fmt_str]
    lea rdx, [rel STR_0]
    call printf

ETIQ_14: ; [MOSTRAR_ID, promedio, -]
    mov ecx, [rel promedio_num]
    mov edx, [rel promedio_den]
    call imprimir_racional

ETIQ_15: ; [>=, promedio, 4/1]
    mov eax, [rel promedio_num]
    mov ebx, [rel promedio_den]
    movsxd r12, eax
    movsxd r13, ebx
    mov eax, 4
    mov ebx, 1
    movsxd r14, eax
    movsxd r15, ebx
    mov rax, r12
    imul rax, r15
    mov rcx, r14
    imul rcx, r13
    cmp rax, rcx
    setge al
    movzx eax, al
    mov [rel T15_num], eax
    mov dword [rel T15_den], 1

ETIQ_16: ; [BF, [T15], [T21]]
    mov eax, [rel T15_num]
    mov ebx, [rel T15_den]
    cmp eax, 0
    je ETIQ_21

ETIQ_17: ; [>=, asistencia, 3/4]
    mov eax, [rel asistencia_num]
    mov ebx, [rel asistencia_den]
    movsxd r12, eax
    movsxd r13, ebx
    mov eax, 3
    mov ebx, 4
    movsxd r14, eax
    movsxd r15, ebx
    mov rax, r12
    imul rax, r15
    mov rcx, r14
    imul rcx, r13
    cmp rax, rcx
    setge al
    movzx eax, al
    mov [rel T17_num], eax
    mov dword [rel T17_den], 1

ETIQ_18: ; [BF, [T17], [T21]]
    mov eax, [rel T17_num]
    mov ebx, [rel T17_den]
    cmp eax, 0
    je ETIQ_21

ETIQ_19: ; [MOSTRAR_STR, El alumno aprueba, -]
    lea rcx, [rel fmt_str]
    lea rdx, [rel STR_1]
    call printf

ETIQ_20: ; [BI, [T22], -]
    jmp ETIQ_22

ETIQ_21: ; [MOSTRAR_STR, El alumno desaprueba, -]
    lea rcx, [rel fmt_str]
    lea rdx, [rel STR_2]
    call printf

ETIQ_22: ; Fin de tercetos

    xor ecx, ecx
    call exit

; ==========================================================
; BIBLIOTECA DE RUTINAS AUXILIARES (MOTOR RACIONAL GRUPO C)
; ==========================================================

normalizar_racional:
    cmp ebx, 0
    je error_div_cero

    cmp eax, 0
    jne .revisar_signo
    mov ebx, 1
    ret

.revisar_signo:
    cmp ebx, 0
    jg .calcular_mcd
    neg eax
    neg ebx

.calcular_mcd:
    mov ecx, eax
    cmp ecx, 0
    jge .mcd_abs_ok
    neg ecx
.mcd_abs_ok:
    mov r8d, ebx

.bucle_euclides:
    cmp r8d, 0
    je .mcd_listo
    push rax
    mov eax, ecx
    cdq
    idiv r8d
    mov ecx, r8d
    mov r8d, edx
    pop rax
    jmp .bucle_euclides

.mcd_listo:
    cdq
    idiv ecx
    mov r9d, eax
    mov eax, ebx
    cdq
    idiv ecx
    mov ebx, eax
    mov eax, r9d
    ret

mcd_64_positivo:
    cmp rax, 0
    jge .m64_a_ok
    neg rax
.m64_a_ok:
    cmp rbx, 0
    jge .m64_b_ok
    neg rbx
.m64_b_ok:
.m64_loop:
    cmp rbx, 0
    je .m64_fin
    cqo
    idiv rbx
    mov rax, rbx
    mov rbx, rdx
    jmp .m64_loop
.m64_fin:
    ret

verificar_overflow_64:
    mov r10, 2147483647
    cmp rax, r10
    jg error_overflow
    mov r10, -2147483648
    cmp rax, r10
    jl error_overflow
    ret

imprimir_racional:
    sub rsp, 40
    mov r8d, edx
    mov edx, ecx
    lea rcx, [rel fmt_rac]
    call printf
    add rsp, 40
    ret

error_div_cero:
    sub rsp, 40
    lea rcx, [rel err_cero]
    call printf
    mov ecx, 1
    call exit

error_overflow:
    sub rsp, 40
    lea rcx, [rel err_ovf]
    call printf
    mov ecx, 1
    call exit
