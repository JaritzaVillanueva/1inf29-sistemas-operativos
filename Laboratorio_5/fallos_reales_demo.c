/* fallos_reales_demo.c
 * Mide fallos de pagina reales usando getrusage().
 * Compilar: gcc -Wall -Wextra fallos_reales_demo.c -o fallos_reales_demo
 * Ejecutar: ./fallos_reales_demo
 */

#include <stdio.h>          /* Para printf. */
#include <stdlib.h>         /* Para exit y EXIT_FAILURE. */
#include <sys/resource.h>   /* Para getrusage y struct rusage. */
#include <sys/mman.h>       /* Para mmap y munmap. */
#include <unistd.h>         /* Para sysconf. */

void imprimir_diferencia(struct rusage antes, struct rusage despues) {
    long fallos_menores;    /* Guardara la diferencia de fallos menores. */
    long fallos_mayores;    /* Guardara la diferencia de fallos mayores. */

    fallos_menores = despues.ru_minflt - antes.ru_minflt;
    fallos_mayores = despues.ru_majflt - antes.ru_majflt;

    printf("Fallos menores: %ld\n", fallos_menores);
    printf("Fallos mayores: %ld\n", fallos_mayores);
}

int main(void) {
    size_t tam_pagina;          /* Tamanio de pagina del sistema. */
    size_t paginas;             /* Cantidad de paginas que vamos a reservar. */
    size_t total_bytes;         /* Tamanio total de la region de memoria. */
    char *memoria;              /* Puntero a la region reservada con mmap. */

    struct rusage antes;        /* Contadores antes de un experimento. */
    struct rusage despues;      /* Contadores despues de un experimento. */

    tam_pagina = (size_t) sysconf(_SC_PAGESIZE); /* Obtiene el tamanio real de pagina. */
    paginas = 51200;                             /* 51200 paginas equivalen a 200 MB si la pagina es 4 KB. */
    total_bytes = paginas * tam_pagina;          /* Calcula el total de bytes a reservar. */

    printf("Tamanio de pagina: %zu bytes\n", tam_pagina);
    printf("Paginas reservadas: %zu\n", paginas);
    printf("Memoria total: %.2f MB\n\n", total_bytes / (1024.0 * 1024.0));

    memoria = mmap(
        NULL,                                    /* El kernel elige la direccion virtual. */
        total_bytes,                             /* Cantidad de bytes a reservar. */
        PROT_READ | PROT_WRITE,                  /* La memoria sera leible y escribible. */
        MAP_PRIVATE | MAP_ANONYMOUS,             /* Memoria anonima privada, no respaldada por archivo. */
        -1,                                      /* No se usa descriptor de archivo. */
        0                                        /* Offset 0 porque no hay archivo. */
    );

    if (memoria == MAP_FAILED) {                 /* Verifica si mmap fallo. */
        perror("mmap");                          /* Imprime el error del sistema. */
        exit(EXIT_FAILURE);                      /* Termina con error. */
    }

    printf("=== Primera pasada secuencial ===\n");

    getrusage(RUSAGE_SELF, &antes);              /* Toma contadores antes de tocar memoria. */

    for (size_t i = 0; i < total_bytes; i += tam_pagina) {
        memoria[i] = 1;                          /* Toca una posicion por pagina. */
    }

    getrusage(RUSAGE_SELF, &despues);            /* Toma contadores despues de tocar memoria. */

    imprimir_diferencia(antes, despues);         /* Imprime fallos generados por la primera pasada. */

    printf("\n=== Segunda pasada secuencial sobre la misma memoria ===\n");

    getrusage(RUSAGE_SELF, &antes);              /* Toma contadores antes de la segunda pasada. */

    for (size_t i = 0; i < total_bytes; i += tam_pagina) {
        memoria[i] = 2;                          /* Vuelve a tocar las mismas paginas. */
    }

    getrusage(RUSAGE_SELF, &despues);            /* Toma contadores despues de la segunda pasada. */

    imprimir_diferencia(antes, despues);         /* Imprime fallos generados por la segunda pasada. */

    munmap(memoria, total_bytes);                /* Libera la region reservada con mmap. */

    return 0;                                    /* Termina correctamente. */
}