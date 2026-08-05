/*
 * ============================================================
 *  MÚLTIPLES PRODUCTORES Y MÚLTIPLES CONSUMIDORES
 *  Terminación por variable compartida (sin centinela)
 *  Laboratorio 3 — 1INF29 Sistemas Operativos · PUCP
 *
 *  Compilación:
 *   gcc -Wall -pthread -o multi_prod_cons multi_prod_cons.c
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>

#define BUFFER_SIZE    5
#define NUM_ITEMS     10   /* ítems que produce CADA productor  */
#define NUM_PRODUCERS  2
#define NUM_CONSUMERS  3

static int buffer[BUFFER_SIZE];
static int indice_escritura = 0;
static int indice_lectura   = 0;

/* Cuántos productores ya terminaron (protegido por sem_mutex) */
static int productores_terminados = 0;
/* Cuántos ítems fueron producidos en total (protegido por sem_mutex) */
static int total_producido  = 0;
/* Cuántos ítems fueron consumidos en total (protegido por sem_mutex) */
static int total_consumido  = 0;

static sem_t sem_empty;
static sem_t sem_full;
static sem_t sem_mutex;

typedef struct { int id; } Args;

/* ============================================================
 *  HILO PRODUCTOR
 * ============================================================ */
void *productor(void *arg) {
    Args *a = (Args *)arg;
    int id  = a->id;

    for (int i = 1; i <= NUM_ITEMS; i++) {
        usleep((rand() % 300 + 100) * 1000);

        sem_wait(&sem_empty);
        sem_wait(&sem_mutex);

        int slot = indice_escritura;
        buffer[indice_escritura] = i;
        indice_escritura = (indice_escritura + 1) % BUFFER_SIZE;
        total_producido++;

        printf("Productor %d  escribió %2d en slot %d\n", id, i, slot);

        sem_post(&sem_mutex);
        sem_post(&sem_full);
    }

    /* Registra que este productor terminó */
    sem_wait(&sem_mutex);
    productores_terminados++;
    sem_post(&sem_mutex);

    printf("Productor %d  Terminó.\n", id);
    return NULL;
}

/* ============================================================
 *  HILO CONSUMIDOR
 * ============================================================ */
void *consumidor(void *arg) {
    Args *a = (Args *)arg;
    int id  = a->id;

    while (1) {
        /* ── Espera NO BLOQUEANTE ─────────────────────────
         *  sem_trywait permite verificar la condición de fin
         *  sin quedar bloqueado cuando el buffer está vacío
         *  y todos los productores ya terminaron.
         * ───────────────────────────────────────────────── */
        if (sem_trywait(&sem_full) != 0) {
            if (errno != EAGAIN) break;

            sem_wait(&sem_mutex);
            int todos_prod_terminaron = (productores_terminados >= NUM_PRODUCERS);
            int items_pendientes      = (total_consumido < total_producido);
            sem_post(&sem_mutex);

            /* Salir solo si todos los productores terminaron
             * Y no quedan ítems pendientes en el buffer     */
            if (todos_prod_terminaron && !items_pendientes) break;

            usleep(50 * 1000);
            continue;
        }

        sem_wait(&sem_mutex);

        int slot = indice_lectura;
        int item = buffer[indice_lectura];
        indice_lectura = (indice_lectura + 1) % BUFFER_SIZE;
        total_consumido++;

        printf("             [CONS %d] leyó   %2d de slot %d  (%d/%d)\n",
               id, item, slot, total_consumido, total_producido);

        sem_post(&sem_mutex);
        sem_post(&sem_empty);

        usleep((rand() % 400 + 200) * 1000);
    }

    printf("             [CONS %d] Terminó.\n", id);
    return NULL;
}

/* ============================================================
 *  MAIN
 * ============================================================ */
int main(void) {
    pthread_t hilos_prod[NUM_PRODUCERS];
    pthread_t hilos_cons[NUM_CONSUMERS];
    Args args_prod[NUM_PRODUCERS];
    Args args_cons[NUM_CONSUMERS];

    printf("=== Multi Prod-Cons | buffer=%d | prod=%d | cons=%d | items/prod=%d ===\n\n",
           BUFFER_SIZE, NUM_PRODUCERS, NUM_CONSUMERS, NUM_ITEMS);

    sem_init(&sem_empty, 0, BUFFER_SIZE);
    sem_init(&sem_full,  0, 0);
    sem_init(&sem_mutex, 0, 1);

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        args_prod[i].id = i + 1;
        pthread_create(&hilos_prod[i], NULL, productor, &args_prod[i]);
    }

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        args_cons[i].id = i + 1;
        pthread_create(&hilos_cons[i], NULL, consumidor, &args_cons[i]);
    }

    for (int i = 0; i < NUM_PRODUCERS; i++)
        pthread_join(hilos_prod[i], NULL);
    for (int i = 0; i < NUM_CONSUMERS; i++)
        pthread_join(hilos_cons[i], NULL);

    sem_destroy(&sem_empty);
    sem_destroy(&sem_full);
    sem_destroy(&sem_mutex);

    printf("\n=== FIN — producidos: %d | consumidos: %d ===\n",
           total_producido, total_consumido);
    return 0;
}