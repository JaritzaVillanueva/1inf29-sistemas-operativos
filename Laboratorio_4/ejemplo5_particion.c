#include <stdio.h>
#include <stdlib.h>

// Estructura que representa un bloque de memoria
typedef struct Bloque {
    int inicio;              // Dirección inicial del bloque
    int tamanio;             // Tamaño del bloque
    struct Bloque *siguiente; // Puntero al siguiente bloque en la lista
} Bloque;

// Lista estática que mantiene todos los bloques disponibles
static Bloque *lista = NULL;

// Función para imprimir todos los bloques de la lista
static void imprimir_lista(const char *etiqueta) {
    printf("%-20s -> ", etiqueta);
    for (Bloque *b = lista; b != NULL; b = b->siguiente) {
        printf("[inicio=%d tam=%d] ", b->inicio, b->tamanio);
    }
    printf("\n");
}

/* Función que parte el primer bloque de la lista en dos "buddies" de la mitad del tamaño */
static void partir_primero(void) {
    Bloque *original = lista;                   // Apunta al primer bloque
    Bloque *buddy = malloc(sizeof(Bloque));    // Crea un nuevo bloque para el buddy
    buddy->tamanio   = original->tamanio / 2;  // Mitad del tamaño del bloque original
    buddy->inicio    = original->inicio + buddy->tamanio; // Dirección inicial del buddy
    buddy->siguiente = original->siguiente;    // El buddy apunta al siguiente bloque de la lista
    original->tamanio   = original->tamanio / 2; // Reducir tamaño del bloque original a la mitad
    original->siguiente = buddy;               // Insertar el buddy justo después del original
}

int main(void) {
    // Inicializar la lista con un único bloque de 1024 KB
    lista = malloc(sizeof(Bloque));
    lista->inicio = 0;
    lista->tamanio = 1024;
    lista->siguiente = NULL;

    // Imprime la lista inicial
    imprimir_lista("inicial");

    // Parte el primer bloque en dos buddies
    partir_primero();
    imprimir_lista("primera particion");

    // Parte nuevamente el primer bloque (ahora de 512 KB) en dos buddies de 256 KB
    partir_primero();
    imprimir_lista("segunda particion");

    // Parte nuevamente el primer bloque (ahora de 256 KB) en dos buddies de 128 KB
    partir_primero();
    imprimir_lista("tercera particion");

    return 0;
}