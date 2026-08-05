/*
 * Dining Philosophers Problem — Implementación en C con pthreads
 * =============================================================
 * Compilar:  gcc -o dining dining_philosophers.c -lpthread
 * Ejecutar:  ./dining
 *
 * Versión 1 (con deadlock)    → todos toman el tenedor izquierdo primero
 * Versión 2 (sin deadlock)    → filósofo N-1 invierte el orden (asimétrico)
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>   /* usleep */
#include <string.h>

#define N           5          /* número de filósofos */
#define EAT_US   300000        /* tiempo comiendo  (µs) */
#define THINK_US 300000        /* tiempo pensando  (µs) */
#define MEALS       3          /* veces que come cada filósofo */

/* ─── Tenedores ────────────────────────────────────────────────────────── */
static pthread_mutex_t forks[N];

/* ─── Argumentos para cada hilo ─────────────────────────────────────────── */
typedef struct {
    int id;
    int *meals;           /* contador compartido (solo para estadísticas) */
} PhilosopherArgs;


/* ═══════════════════════════════════════════════════════════════════════════
 *  VERSIÓN 1: CON DEADLOCK
 *  Todos toman primero el tenedor izquierdo (id) y luego el derecho (id+1).
 *  Si los N hilos lo hacen simultáneamente forman un ciclo de espera.
 * ═══════════════════════════════════════════════════════════════════════════ */
void *philosopher_deadlock(void *arg)
{
    PhilosopherArgs *a = (PhilosopherArgs *)arg;
    int id    = a->id;
    int left  = id;
    int right = (id + 1) % N;

    printf("[Filósofo %d] Esperando tenedor izquierdo (%d)...\n", id, left);
    pthread_mutex_lock(&forks[left]);
    printf("[Filósofo %d] ✔ Tomó tenedor izquierdo (%d)\n", id, left);

    usleep(50000);   /* pequeña pausa para que el deadlock sea reproducible */

    printf("[Filósofo %d] Esperando tenedor derecho  (%d)...\n", id, right);
    pthread_mutex_lock(&forks[right]);   /* ← bloqueado si hay deadlock */

    /* Si llegamos aquí (no debería con deadlock): */
    printf("[Filósofo %d] 🍝 Comiendo...\n", id);
    usleep(EAT_US);

    pthread_mutex_unlock(&forks[right]);
    pthread_mutex_unlock(&forks[left]);
    return NULL;
}


/* ═══════════════════════════════════════════════════════════════════════════
 *  VERSIÓN 2: SIN DEADLOCK — Filósofo asimétrico
 *  El filósofo N-1 toma primero el derecho y luego el izquierdo.
 *  Esto rompe el ciclo: nunca todos esperan en la misma dirección.
 * ═══════════════════════════════════════════════════════════════════════════ */
void *philosopher_safe(void *arg)
{
    PhilosopherArgs *a = (PhilosopherArgs *)arg;
    int id    = a->id;
    int left  = id;
    int right = (id + 1) % N;

    /* El último filósofo invierte el orden de adquisición */
    int first  = (id == N - 1) ? right : left;
    int second = (id == N - 1) ? left  : right;

    for (int i = 0; i < MEALS; i++) {

        /* ── Pensar ───────────────────────────────────── */
        printf("[Filósofo %d] 💭 Pensando...\n", id);
        usleep(THINK_US / 3 + rand() % (THINK_US / 2));

        /* ── Tomar tenedores ──────────────────────────── */
        pthread_mutex_lock(&forks[first]);
        printf("[Filósofo %d] 🍴 Tomó tenedor %d\n", id, first);

        pthread_mutex_lock(&forks[second]);
        printf("[Filósofo %d] 🍴 Tomó tenedor %d\n", id, second);

        /* ── Comer ────────────────────────────────────── */
        a->meals[id]++;
        printf("[Filósofo %d] 🍝 Comiendo... (comida #%d)\n", id, a->meals[id]);
        usleep(EAT_US / 3 + rand() % (EAT_US / 2));

        /* ── Soltar tenedores ─────────────────────────── */
        pthread_mutex_unlock(&forks[second]);
        pthread_mutex_unlock(&forks[first]);
        printf("[Filósofo %d] ↩  Soltó tenedores %d y %d\n", id, second, first);
    }

    printf("[Filósofo %d] ✅ Terminó (%d comidas)\n", id, a->meals[id]);
    return NULL;
}


/* ─── Helpers ────────────────────────────────────────────────────────────── */
static void init_forks(void)
{
    for (int i = 0; i < N; i++)
        pthread_mutex_init(&forks[i], NULL);
}

static void destroy_forks(void)
{
    for (int i = 0; i < N; i++)
        pthread_mutex_destroy(&forks[i]);
}


/* ═══════════════════════════════════════════════════════════════════════════
 *  DEMO 1: Deadlock
 * ═══════════════════════════════════════════════════════════════════════════ */
void demo_deadlock(void)
{
    printf("\n============================================================\n");
    printf("  VERSIÓN 1: CON DEADLOCK (espera 2 s y continúa)\n");
    printf("============================================================\n");

    init_forks();

    pthread_t      threads[N];
    PhilosopherArgs args[N];
    int meals[N];
    memset(meals, 0, sizeof(meals));

    for (int i = 0; i < N; i++) {
        args[i].id    = i;
        args[i].meals = meals;
        pthread_create(&threads[i], NULL, philosopher_deadlock, &args[i]);
    }

    /* Esperamos 2 s; los hilos son daemon-like — los abandonamos */
    sleep(2);

    /* Comprobamos cuántos siguen bloqueados */
    int blocked = 0;
    for (int i = 0; i < N; i++) {
        /* trylock sobre cada tenedor: si falla, sigue tomado → bloqueado */
        if (pthread_mutex_trylock(&forks[i]) != 0) {
            blocked++;
        } else {
            pthread_mutex_unlock(&forks[i]);
        }
    }

    if (blocked > 0)
        printf("\n⚠️  DEADLOCK detectado: %d tenedor(es) siguen bloqueados.\n\n", blocked);
    else
        printf("\n✅ Terminaron todos (poco probable con esta versión).\n\n");

    /*
     * Nota: los hilos bloqueados se cancelan al salir del proceso.
     * En producción usaríamos pthread_cancel + pthread_timedjoin_np.
     */
    destroy_forks();
}


/* ═══════════════════════════════════════════════════════════════════════════
 *  DEMO 2: Sin deadlock
 * ═══════════════════════════════════════════════════════════════════════════ */
void demo_safe(void)
{
    printf("\n============================================================\n");
    printf("  VERSIÓN 2: SIN DEADLOCK — Filósofo asimétrico\n");
    printf("============================================================\n");

    init_forks();
    srand(42);

    pthread_t       threads[N];
    PhilosopherArgs args[N];
    int meals[N];
    memset(meals, 0, sizeof(meals));

    for (int i = 0; i < N; i++) {
        args[i].id    = i;
        args[i].meals = meals;
        pthread_create(&threads[i], NULL, philosopher_safe, &args[i]);
    }

    for (int i = 0; i < N; i++)
        pthread_join(threads[i], NULL);

    printf("\n📊 Resumen de comidas:\n");
    for (int i = 0; i < N; i++)
        printf("   Filósofo %d: %d comidas\n", i, meals[i]);
    printf("\n");

    destroy_forks();
}


/* ─── main ───────────────────────────────────────────────────────────────── */
int main(void)
{
    demo_deadlock();
    demo_safe();
    return 0;
}