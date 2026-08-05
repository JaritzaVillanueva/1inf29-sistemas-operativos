/*
 * ============================================================
 *  CENA DE LOS FILÓSOFOS
 *  Laboratorio 3 — 1INF29 Sistemas Operativos · PUCP
 *
 *  Problema:
 *   5 filósofos sentados en una mesa circular. Entre cada par
 *   hay un tenedor (5 en total). Para comer, un filósofo
 *   necesita el tenedor de su izquierda Y el de su derecha.
 *   Cuando no come, piensa.
 *
 *  Riesgo sin solución:
 *   Si todos toman el tenedor izquierdo al mismo tiempo,
 *   nadie puede tomar el derecho → DEADLOCK.
 *
 *  Solución aplicada — asimetría:
 *   Los filósofos 0..3 toman primero izquierda, luego derecha.
 *   El filósofo 4 toma primero derecha, luego izquierda.
 *   Esto rompe la condición de espera circular que causa deadlock.
 *
 *  Semáforos usados:
 *   tenedor[i] → semáforo binario por cada tenedor (inicia en 1)
 *                un filósofo lo toma con sem_wait y lo suelta con sem_post
 *
 *  Compilación:
 *   gcc -Wall -pthread -o filosofos filosofos.c
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

/* ----------------------------------------------------------
 * PARÁMETROS
 * ---------------------------------------------------------- */
#define N        5   /* número de filósofos (y de tenedores) */
#define RONDAS   3   /* cuántas veces come cada filósofo     */

/* ----------------------------------------------------------
 * TENEDORES
 * Cada tenedor es un semáforo binario (inicia en 1 = disponible).
 * tenedor[i] está entre el filósofo i y el filósofo (i+1)%N.
 * Un filósofo hace sem_wait para tomarlo y sem_post para soltarlo.
 * ---------------------------------------------------------- */
static sem_t tenedor[N];

/* ----------------------------------------------------------
 * IDENTIFICADOR DE CADA HILO FILÓSOFO
 * ---------------------------------------------------------- */
typedef struct { int id; } FilosofoArgs;

/* ----------------------------------------------------------
 * MACROS DE TENEDORES
 * El tenedor izquierdo del filósofo i es el tenedor i.
 * El tenedor derecho  del filósofo i es el tenedor (i+1)%N.
 * ---------------------------------------------------------- */
#define IZQUIERDA(i)  (i)
#define DERECHA(i)    ((i + 1) % N)

/* ============================================================
 *  HILO FILÓSOFO
 * ============================================================ */
void *filosofo(void *arg) {
    FilosofoArgs *fa = (FilosofoArgs *)arg;
    int id = fa->id;

    for (int ronda = 1; ronda <= RONDAS; ronda++) {

        /* ── FASE PENSAR ────────────────────────────────── */
        printf("Filósofo %d  [ronda %d]  pensando...\n", id, ronda);
        usleep((rand() % 400 + 200) * 1000);  /* 200-600 ms */

        /* ── INTENTAR TOMAR TENEDORES ────────────────────
         *
         *  SOLUCIÓN ASIMÉTRICA para evitar deadlock:
         *
         *  Deadlock ocurre cuando TODOS los filósofos toman
         *  su tenedor izquierdo simultáneamente: cada uno
         *  espera el derecho que su vecino ya tomó → ciclo
         *  de espera circular → nadie avanza nunca.
         *
         *  La solución: el filósofo N-1 (el último) invierte
         *  el orden — toma primero el derecho, luego el izquierdo.
         *  Esto rompe la simetría y garantiza que al menos
         *  un filósofo siempre pueda tomar ambos tenedores.
         * ───────────────────────────────────────────────── */
        if (id < N - 1) {
            /* Filósofos 0..3: primero izquierda, luego derecha */
            printf("Filósofo %d  esperando tenedor izquierdo (%d)...\n",
                   id, IZQUIERDA(id));
            sem_wait(&tenedor[IZQUIERDA(id)]);

            printf("Filósofo %d  esperando tenedor derecho   (%d)...\n",
                   id, DERECHA(id));
            sem_wait(&tenedor[DERECHA(id)]);
        } else {
            /* Filósofo 4: primero derecha, luego izquierda
             * Este es el único cambio que evita el deadlock. */
            printf("Filósofo %d  esperando tenedor derecho   (%d)...\n",
                   id, DERECHA(id));
            sem_wait(&tenedor[DERECHA(id)]);

            printf("Filósofo %d  esperando tenedor izquierdo (%d)...\n",
                   id, IZQUIERDA(id));
            sem_wait(&tenedor[IZQUIERDA(id)]);
        }

        /* ── FASE COMER ──────────────────────────────────
         * El filósofo tiene ambos tenedores: puede comer.  */
        printf("Filósofo %d  *** COMIENDO *** (tenedores %d y %d)\n",
               id, IZQUIERDA(id), DERECHA(id));
        usleep((rand() % 400 + 200) * 1000);  /* 200-600 ms */

        /* ── SOLTAR TENEDORES ────────────────────────────
         * sem_post devuelve el tenedor al estado disponible (1).
         * Esto puede despertar a un filósofo vecino que
         * estaba bloqueado esperando ese tenedor.          */
        sem_post(&tenedor[IZQUIERDA(id)]);
        sem_post(&tenedor[DERECHA(id)]);

        printf("Filósofo %d  soltó tenedores %d y %d\n",
               id, IZQUIERDA(id), DERECHA(id));
    }

    printf("Filósofo %d  Terminó sus %d rondas.\n", id, RONDAS);
    return NULL;
}

/* ============================================================
 *  MAIN
 * ============================================================ */
int main(void) {
    pthread_t hilos[N];
    FilosofoArgs args[N];

    printf("=== Cena de los Filósofos | N=%d filósofos | %d rondas ===\n\n",
           N, RONDAS);

    /* Inicializar un semáforo binario por cada tenedor.
     * Valor inicial = 1 → tenedor disponible (nadie lo tiene).
     * pshared = 0 → compartido entre hilos del mismo proceso.  */
    for (int i = 0; i < N; i++)
        sem_init(&tenedor[i], 0, 1);

    /* Crear un hilo por cada filósofo */
    for (int i = 0; i < N; i++) {
        args[i].id = i;
        pthread_create(&hilos[i], NULL, filosofo, &args[i]);
    }

    /* Esperar que todos los filósofos terminen */
    for (int i = 0; i < N; i++)
        pthread_join(hilos[i], NULL);

    /* Destruir semáforos */
    for (int i = 0; i < N; i++)
        sem_destroy(&tenedor[i]);

    printf("\n=== FIN — Ningún deadlock ocurrió. ===\n");
    return 0;
}