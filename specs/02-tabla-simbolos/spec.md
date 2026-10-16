# Spec — 02-tabla-simbolos
# Especificación Técnica: Tabla de Símbolos

## 1. Descripción General
La **Tabla de Símbolos (TS)** es la estructura de datos en memoria encargada de almacenar
y gestionar la información relevante sobre los identificadores (`ID`) y constantes de texto (`LIT_CADENA`) o numéricas (`LIT_RACIONAL`) reconocidas por el analizador léxico durante 
la lectura del código fuente en lenguaje Razio.

---

## 2. Estructura de Datos en C (`RegTablaSimbolos`)

En el archivo `alexico.c`, la Tabla de Símbolos está implementada mediante un arreglo estático de estructuras (`struct`) con una capacidad máxima de 500 registros:

```c
#define MAX_BUFFER 256

typedef struct {
    char nombre[MAX_BUFFER];  /* Lexema original o valor textual */
    char tipo[20];            /* Categoría: "ID", "LIT_RACIONAL" o "LIT_CADENA" */
    char valor[MAX_BUFFER];   /* Valor asignado o valor racional normalizado */
    int longitud;             /* Largo en caracteres del lexema */
} RegTablaSimbolos;

/* Arreglo global de la Tabla de Símbolos */
RegTablaSimbolos tabla_simbolos[500];
int cant_simbolos = 0;







## Objetivo

## Alcance

## Entradas / Salidas

## Reglas y decisiones de diseño

## Criterios de aceptación
