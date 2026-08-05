/* problema_lector_escritor.c
 * Múltiples lectores pueden leer simultáneamente.
 * Solo un escritor accede a la vez, y excluye a todos los lectores.
 * Compilar: gcc escritores-lectores.c -o lector_escritor -lrt -lpthread
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <semaphore.h>

#define SHM_NOMBRE      "/le_shm"
#define SEM_ESCRITURA   "/le_escritura"  /* Excluye escritores y lectores  */
#define SEM_MUTEX       "/le_mutex"      /* Protege el contador de lectores */
#define NUM_LECTORES    3
#define NUM_ESCRITORES  2
#define ITERACIONES     4

/* ── Estructura en memoria compartida ── */
typedef struct {
    int dato;           /* Recurso compartido que se lee/escribe  */
    int num_lectores;   /* Cuántos lectores están leyendo ahora   */
} RecursoCompartido;

int main()
{
    /* ── 1. Memoria compartida ── */
    int fd = shm_open(SHM_NOMBRE, O_CREAT | O_RDWR, 0666);
    if (fd == -1) { perror("shm_open"); exit(EXIT_FAILURE); }

    ftruncate(fd, sizeof(RecursoCompartido));

    RecursoCompartido *shm = mmap(NULL, sizeof(RecursoCompartido),
                                  PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm == MAP_FAILED) { perror("mmap"); exit(EXIT_FAILURE); }
    close(fd);

    shm->dato         = 0;
    shm->num_lectores = 0;

    /* ── 2. Semáforos ─────────────────────────────────────────────────────
     *
     *  escritura : valor 1 → mutex entre escritores, y entre escritor/lector
     *  mutex     : valor 1 → protege num_lectores (solo lectores lo usan)
     */
    sem_t *escritura = sem_open(SEM_ESCRITURA, O_CREAT, 0666, 1);
    sem_t *mutex     = sem_open(SEM_MUTEX,     O_CREAT, 0666, 1);

    if (escritura == SEM_FAILED || mutex == SEM_FAILED) {
        perror("sem_open"); exit(EXIT_FAILURE);
    }

    /* ── 3. Crear procesos lectores ── */
    for (int i = 0; i < NUM_LECTORES; i++)
    {
        pid_t pid = fork();
        if (pid == -1) { perror("fork"); exit(EXIT_FAILURE); }

        if (pid == 0)
        {
            /* ════════════════════════════════════════════
             *  LECTOR
             *
             *  Protocolo de entrada:
             *    - Incrementa num_lectores con mutex
             *    - Si es el PRIMER lector → bloquea escritura
             *    - Libera mutex
             *
             *  Protocolo de salida:
             *    - Decrementa num_lectores con mutex
             *    - Si es el ÚLTIMO lector → libera escritura
             *    - Libera mutex
             * ════════════════════════════════════════════ */
            int id = i + 1;

            for (int j = 0; j < ITERACIONES; j++)
            {
                /* ── Entrada ── */
                sem_wait(mutex);
                shm->num_lectores++;
                if (shm->num_lectores == 1)
                    sem_wait(escritura);    /* Primer lector bloquea escritores */
                sem_post(mutex);

                /* ── Sección de lectura (varios lectores simultáneos) ── */
                printf("  Lector  %d leyó:     dato = %d  (lectores activos: %d)\n",
                       id, shm->dato, shm->num_lectores);
                usleep(60000);

                /* ── Salida ── */
                sem_wait(mutex);
                shm->num_lectores--;
                if (shm->num_lectores == 0)
                    sem_post(escritura);    /* Último lector libera escritores  */
                sem_post(mutex);

                usleep(40000);
            }

            printf("  Lector  %d terminó.\n", id);
            sem_close(escritura);
            sem_close(mutex);
            munmap(shm, sizeof(RecursoCompartido));
            exit(0);
        }
    }

    /* ── 4. Crear procesos escritores ── */
    for (int i = 0; i < NUM_ESCRITORES; i++)
    {
        pid_t pid = fork();
        if (pid == -1) { perror("fork"); exit(EXIT_FAILURE); }

        if (pid == 0)
        {
            /* ════════════════════════════════════════════
             *  ESCRITOR
             *
             *  Protocolo: mucho más simple que el lector.
             *    - sem_wait(escritura) → acceso exclusivo total
             *    - escribe
             *    - sem_post(escritura) → libera
             *
             *  Mientras un escritor tiene escritura,
             *  ningún otro escritor NI ningún lector nuevo
             *  puede entrar.
             * ════════════════════════════════════════════ */
            int id = i + 1;

            for (int j = 0; j < ITERACIONES; j++)
            {
                sem_wait(escritura);        /* Acceso exclusivo        */

                shm->dato++;
                printf("Escritor %d escribió:  dato = %d\n", id, shm->dato);
                usleep(80000);

                sem_post(escritura);        /* Libera acceso           */

                usleep(100000);
            }

            printf("Escritor %d terminó.\n", id);
            sem_close(escritura);
            sem_close(mutex);
            munmap(shm, sizeof(RecursoCompartido));
            exit(0);
        }
    }

    /* ── 5. Padre espera a todos los hijos ── */
    for (int i = 0; i < NUM_LECTORES + NUM_ESCRITORES; i++)
        wait(NULL);

    printf("\nTodos terminaron. Dato final: %d\n", shm->dato);
    printf("Valor esperado:              %d\n",
           NUM_ESCRITORES * ITERACIONES);

    /* ── 6. Limpieza ── */
    sem_close(escritura);   sem_unlink(SEM_ESCRITURA);
    sem_close(mutex);       sem_unlink(SEM_MUTEX);
    munmap(shm, sizeof(RecursoCompartido));
    shm_unlink(SHM_NOMBRE);

    return 0;
}