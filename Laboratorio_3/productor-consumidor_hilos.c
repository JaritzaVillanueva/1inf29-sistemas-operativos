/*
 * ============================================================
 *  PRODUCTOR-CONSUMIDOR CON MÚLTIPLES CONSUMIDORES
 *  Modelo:
 *   1 productor  →  buffer circular (capacidad N)  →  C consumidores
 *   Cualquier consumidor libre toma el siguiente ítem disponible
 *   (intercambiables: no hay asignación fija de ítems).
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>   /* usleep() */
#include <errno.h>    /* EAGAIN   */

/* ── Parámetros configurables ─────────────────────────────── */
#define BUFFER_SIZE    5   /* capacidad del buffer circular      */
#define NUM_ITEMS     20   /* total de ítems que genera el prod. */
#define NUM_CONSUMERS  3   /* número de consumidores             */
/* ─────────────────────────────────────────────────────────── */

/* ── Buffer circular compartido entre todos los hilos ─────── */
static int buffer[BUFFER_SIZE];
static int in  = 0;   /* índice de escritura (productor)  */
static int out = 0;   /* índice de lectura   (consumidores)*/

/* ── Contador global de ítems consumidos ─────────────────── */
static int total_consumido = 0;

/* ── Semáforos NO NOMBRADOS (sem_init / sem_destroy) ─────────
 *
 *  sem_empty → cuenta celdas VACÍAS disponibles para escribir
 *              valor inicial = BUFFER_SIZE (todo vacío al inicio)
 *
 *  sem_full  → cuenta celdas LLENAS disponibles para leer
 *              valor inicial = 0 (nada producido aún)
 *
 *  sem_mutex → exclusión mutua sobre buffer y total_consumido
 *              valor inicial = 1 (mutex libre = binario)
 * ─────────────────────────────────────────────────────────── */
static sem_t sem_empty;
static sem_t sem_full;
static sem_t sem_mutex;

/* ── Argumento que recibe cada hilo consumidor ────────────── */
typedef struct { int id; } ConsumerArgs;

/* ============================================================
 *  HILO PRODUCTOR
 * ============================================================ */
void *productor(void *arg) {
    (void)arg;

    for (int i = 1; i <= NUM_ITEMS; i++) {
        int item = i;

        /* Simula tiempo variable de producción (100-400 ms) */
        usleep((rand() % 300 + 100) * 1000);

        /* ── Espera BLOQUEANTE ────────────────────────────
         *  sem_wait() decrementa sem_empty; si vale 0 (buffer
         *  lleno) suspende el hilo hasta que un consumidor
         *  libere un slot.
         *  Elegimos bloqueante porque el productor no tiene
         *  nada útil que hacer mientras el buffer esté lleno.
         * ───────────────────────────────────────────────── */
        sem_wait(&sem_empty);

        /* ── Sección crítica: escribir en el buffer ──────
         *  ORDEN obligatorio: sem_empty/sem_full ANTES que mutex.
         *  Invertirlo cuando el buffer está lleno provoca deadlock.
         * ───────────────────────────────────────────────── */
        sem_wait(&sem_mutex);

        buffer[in] = item;
        printf("[PROD]      item=%2d  → buffer[%d]\n", item, in);
        in = (in + 1) % BUFFER_SIZE;

        sem_post(&sem_mutex);

        /* Señala que hay un slot lleno más */
        sem_post(&sem_full);
    }

    printf("[PROD]      Terminó. %d ítems producidos.\n", NUM_ITEMS);
    return NULL;
}

/* ============================================================
 *  HILO CONSUMIDOR (intercambiable: cualquiera toma el siguiente ítem)
 * ============================================================ */
void *consumidor(void *arg) {
    ConsumerArgs *ca = (ConsumerArgs *)arg;
    int id = ca->id;

    while (1) {
        /* ── Espera NO BLOQUEANTE ─────────────────────────
         *  sem_trywait() intenta decrementar sem_full:
         *   retorna  0  → éxito, hay ítem disponible
         *   retorna -1  → sem_full==0 (buffer vacío), errno=EAGAIN
         *
         *  Usamos sem_trywait (no bloqueante) aquí porque
         *  necesitamos verificar la condición de fin sin
         *  quedar suspendidos indefinidamente cuando el
         *  productor ya terminó y el buffer quedó vacío.
         *
         *  Si no hubiera condición de fin, lo correcto sería
         *  sem_wait() (bloqueante) para no desperdiciar CPU.
         * ───────────────────────────────────────────────── */
        if (sem_trywait(&sem_full) != 0) {
            if (errno != EAGAIN) break;   /* error inesperado */

            /* Buffer vacío: ¿ya se consumió todo? */
            sem_wait(&sem_mutex);
            int listo = (total_consumido >= NUM_ITEMS);
            sem_post(&sem_mutex);

            if (listo) break;

            /* Pequeña pausa antes de reintentar (evita busy-wait puro) */
            usleep(50 * 1000);
            continue;
        }

        /* ── Sección crítica: retirar ítem del buffer ──── */
        sem_wait(&sem_mutex);

        int slot = out;
        int item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        total_consumido++;

        printf("         [CONS %d]  item=%2d  ← buffer[%d]  (%d/%d)\n",
               id, item, slot, total_consumido, NUM_ITEMS);

        sem_post(&sem_mutex);

        /* Señala que hay un slot vacío más */
        sem_post(&sem_empty);

        /* Simula tiempo variable de procesamiento (200-600 ms) */
        usleep((rand() % 400 + 200) * 1000);
    }

    printf("         [CONS %d]  Terminó.\n", id);
    return NULL;
}

/* ============================================================
 *  MAIN
 * ============================================================ */
int main(void) {
    pthread_t hilo_prod;
    pthread_t hilos_cons[NUM_CONSUMERS];
    ConsumerArgs args[NUM_CONSUMERS];

    printf("=== Prod-Cons | buffer=%d | consumidores=%d | items=%d ===\n\n",
           BUFFER_SIZE, NUM_CONSUMERS, NUM_ITEMS);

    /* ── Inicializar semáforos NO NOMBRADOS ──────────────────
     *
     *  sem_init(sem, pshared, valor_inicial)
     *  pshared = 0 → compartido entre hilos del mismo proceso
     *  pshared = 1 → entre procesos distintos (necesita shm)
     *
     *  Diferencia con sem_open (nombrado):
     *   sem_init  → vive en memoria del proceso, sin nombre en /dev/shm
     *   sem_open  → tiene nombre en el FS virtual, visible entre procesos
     *  Usamos sem_init porque todos los hilos comparten el mismo proceso.
     * ─────────────────────────────────────────────────────── */
    sem_init(&sem_empty, 0, BUFFER_SIZE);
    sem_init(&sem_full,  0, 0);
    sem_init(&sem_mutex, 0, 1);

    /* ── Crear hilo productor ─────────────────────────────── */
    pthread_create(&hilo_prod, NULL, productor, NULL);

    /* ── Crear hilos consumidores ─────────────────────────── */
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        args[i].id = i + 1;
        pthread_create(&hilos_cons[i], NULL, consumidor, &args[i]);
    }

    /* ── Esperar que todos los hilos terminen ─────────────── */
    pthread_join(hilo_prod, NULL);
    for (int i = 0; i < NUM_CONSUMERS; i++)
        pthread_join(hilos_cons[i], NULL);

    /* ── Destruir semáforos ───────────────────────────────── */
    sem_destroy(&sem_empty);
    sem_destroy(&sem_full);
    sem_destroy(&sem_mutex);

    printf("\n=== FIN — Items consumidos: %d / %d ===\n",
           total_consumido, NUM_ITEMS);
    return 0;
}