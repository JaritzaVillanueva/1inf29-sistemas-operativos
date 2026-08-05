/* problema2_segmento_simple.c
 * Productor-consumidor con un solo slot compartido.
 * No necesita struct ni índices — los semáforos coordinan todo.
 * Compilar: gcc problema2_segmento_simple.c -o prob2_simple -lrt -lpthread
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <semaphore.h>

#define SHM_NOMBRE  "/prob2_simple_shm"
#define SEM_LLENO   "/prob2_simple_lleno"  /* slot tiene dato listo    */
#define SEM_VACIO   "/prob2_simple_vacio"  /* slot está libre          */
#define ITEMS       8

int main()
{
    /* ── 1. Segmento compartido: solo un entero ── */
    int fd = shm_open(SHM_NOMBRE, O_CREAT | O_RDWR, 0666);
    if (fd == -1) { perror("shm_open"); exit(EXIT_FAILURE); }

    ftruncate(fd, sizeof(int));

    int *slot = mmap(NULL, sizeof(int),
                     PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (slot == MAP_FAILED) { perror("mmap"); exit(EXIT_FAILURE); }
    close(fd);

    /* ── 2. Solo DOS semáforos — no necesitamos mutex ──────────────────
     *
     *  Sin buffer circular no hay índices que proteger con mutex.
     *  El slot solo lo toca uno a la vez por diseño:
     *    - Productor escribe SOLO cuando vacio > 0
     *    - Consumidor lee   SOLO cuando lleno > 0
     *  Nunca acceden al slot al mismo tiempo → no hay sección crítica.
     */
    sem_t *lleno = sem_open(SEM_LLENO, O_CREAT, 0666, 0); /* slot vacío al inicio */
    sem_t *vacio = sem_open(SEM_VACIO, O_CREAT, 0666, 1); /* 1 slot libre         */

    if (lleno == SEM_FAILED || vacio == SEM_FAILED) {
        perror("sem_open"); exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) { perror("fork"); exit(EXIT_FAILURE); }

    /* ════════════════════════════════════════════════
     *  HIJO — CONSUMIDOR
     * ════════════════════════════════════════════════ */
    if (pid == 0)
    {
        for (int i = 0; i < ITEMS; i++)
        {
            sem_wait(lleno);        /* Espera dato disponible   */

            int dato = *slot;       /* Lee el slot              */
            printf("  Consumidor leyó:     %d\n", dato);

            sem_post(vacio);        /* Slot libre de nuevo      */
            usleep(80000);
        }

        printf("Consumidor: terminó.\n");
        sem_close(lleno);
        sem_close(vacio);
        munmap(slot, sizeof(int));
        exit(0);
    }

    /* ════════════════════════════════════════════════
     *  PADRE — PRODUCTOR
     * ════════════════════════════════════════════════ */
    for (int i = 1; i <= ITEMS; i++)
    {
        sem_wait(vacio);            /* Espera slot libre        */

        *slot = i * 10;             /* Escribe en el slot       */
        printf("Productor  escribió:  %d\n", *slot);

        sem_post(lleno);            /* Dato listo para consumir */
        usleep(50000);
    }

    printf("Productor: terminó.\n");

    wait(NULL);

    sem_close(lleno);   sem_unlink(SEM_LLENO);
    sem_close(vacio);   sem_unlink(SEM_VACIO);
    munmap(slot, sizeof(int));
    shm_unlink(SHM_NOMBRE);

    printf("Recursos liberados.\n");
    return 0;
}