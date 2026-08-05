/* filosofos.c
 * 5 filósofos, 5 tenedores. Cada filósofo necesita el tenedor
 * izquierdo y derecho para comer.
 * Compilar: gcc filosofos.c -o filosofos -lrt -lpthread
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <unistd.h>

#define N           5
#define ITERACIONES 3

/* Nombres para los 5 semáforos (uno por tenedor) */
const char *SEM_TENEDOR[] = {
    "/fil_tenedor0", "/fil_tenedor1", "/fil_tenedor2",
    "/fil_tenedor3", "/fil_tenedor4"
};

#define IZQUIERDO(i) (i)
#define DERECHO(i)   ((i + 1) % N)

int main()
{
    /* ── 1. Crear un semáforo por tenedor, valor inicial 1 (disponible) ── */
    sem_t *tenedor[N];
    for (int i = 0; i < N; i++)
    {
        tenedor[i] = sem_open(SEM_TENEDOR[i], O_CREAT, 0666, 1);
        if (tenedor[i] == SEM_FAILED) { perror("sem_open"); exit(EXIT_FAILURE); }
    }

    /* ── 2. Crear un proceso por filósofo ── */
    for (int i = 0; i < N; i++)
    {
        pid_t pid = fork();
        if (pid == -1) { perror("fork"); exit(EXIT_FAILURE); }

        if (pid == 0)
        {
            /* ════════════════════════════════════════════════════
             *  FILÓSOFO i
             *
             *  Solución asimétrica para evitar deadlock:
             *  - Filósofos PARES    toman izquierdo → derecho
             *  - Filósofos IMPARES  toman derecho   → izquierdo
             *
             *  Si todos tomaran primero el izquierdo, cada uno
             *  agarraría su tenedor y esperaría el del vecino
             *  → deadlock circular garantizado.
             * ════════════════════════════════════════════════════ */
            for (int j = 0; j < ITERACIONES; j++)
            {
                /* ── Pensar ── */
                printf("Filósofo %d está pensando...\n", i);
                usleep(50000 + rand() % 50000);

                /* ── Tomar tenedores (orden asimétrico) ── */
                if (i % 2 == 0)
                {
                    sem_wait(tenedor[IZQUIERDO(i)]);
                    printf("Filósofo %d tomó tenedor IZQUIERDO %d\n",
                           i, IZQUIERDO(i));
                    sem_wait(tenedor[DERECHO(i)]);
                    printf("Filósofo %d tomó tenedor DERECHO   %d\n",
                           i, DERECHO(i));
                }
                else
                {
                    sem_wait(tenedor[DERECHO(i)]);
                    printf("Filósofo %d tomó tenedor DERECHO   %d\n",
                           i, DERECHO(i));
                    sem_wait(tenedor[IZQUIERDO(i)]);
                    printf("Filósofo %d tomó tenedor IZQUIERDO %d\n",
                           i, IZQUIERDO(i));
                }

                /* ── Comer ── */
                printf("*** Filósofo %d está COMIENDO (iter %d) ***\n",
                       i, j + 1);
                usleep(80000 + rand() % 40000);

                /* ── Soltar tenedores ── */
                sem_post(tenedor[IZQUIERDO(i)]);
                sem_post(tenedor[DERECHO(i)]);
                printf("Filósofo %d soltó sus tenedores.\n", i);
            }

            printf("Filósofo %d terminó.\n", i);
            for (int k = 0; k < N; k++) sem_close(tenedor[k]);
            exit(0);
        }
    }

    /* ── 3. Padre espera a todos ── */
    for (int i = 0; i < N; i++) wait(NULL);

    /* ── 4. Limpieza ── */
    for (int i = 0; i < N; i++)
    {
        sem_close(tenedor[i]);
        sem_unlink(SEM_TENEDOR[i]);
    }

    printf("\nTodos los filósofos terminaron.\n");
    return 0;
}