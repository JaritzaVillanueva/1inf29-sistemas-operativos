/* multi_prod_cons.c
 * 2 productores y 2 consumidores sobre el mismo buffer circular.
 * El mutex ahora es imprescindible porque dos productores
 * pueden intentar escribir en 'in' al mismo tiempo.
 * Compilar: gcc multi_prod_cons.c -o multi_pc -lrt -lpthread
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <unistd.h>

#define SHM_NOMBRE    "/mpc_shm"
#define SEM_MUTEX     "/mpc_mutex"
#define SEM_LLENO     "/mpc_lleno"
#define SEM_VACIO     "/mpc_vacio"
#define N             4       /* Slots del buffer           */
#define PRODUCTORES   2
#define CONSUMIDORES  2
#define ITEMS         6       /* Ítems que produce cada uno */

typedef struct {
    int buffer[N];
    int in;
    int out;
} Buffer;

int main()
{
    /* ── 1. Memoria compartida ── */
    int fd = shm_open(SHM_NOMBRE, O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(Buffer));
    Buffer *shm = mmap(NULL, sizeof(Buffer),
                       PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    shm->in = shm->out = 0;

    /* ── 2. Semáforos ── */
    sem_t *mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);
    sem_t *lleno = sem_open(SEM_LLENO, O_CREAT, 0666, 0);
    sem_t *vacio = sem_open(SEM_VACIO, O_CREAT, 0666, N);

    /* ── 3. Lanzar productores ── */
    for (int i = 0; i < PRODUCTORES; i++)
    {
        if (fork() == 0)
        {
            for (int j = 0; j < ITEMS; j++)
            {
                int dato = (i + 1) * 100 + j;

                sem_wait(vacio);
                sem_wait(mutex);        /* ← crítico: dos productores    */
                                        /*   compiten por shm->in        */
                shm->buffer[shm->in] = dato;
                shm->in = (shm->in + 1) % N;
                sem_post(mutex);
                sem_post(lleno);

                printf("Productor %d produjo:  %d\n", i + 1, dato);
                usleep(50000);
            }
            printf("Productor %d terminó.\n", i + 1);
            sem_close(mutex); sem_close(lleno); sem_close(vacio);
            munmap(shm, sizeof(Buffer));
            exit(0);
        }
    }

    /* ── 4. Lanzar consumidores ── */
    for (int i = 0; i < CONSUMIDORES; i++)
    {
        if (fork() == 0)
        {
            int total = PRODUCTORES * ITEMS / CONSUMIDORES;
            for (int j = 0; j < total; j++)
            {
                sem_wait(lleno);
                sem_wait(mutex);        /* ← crítico: dos consumidores   */
                                        /*   compiten por shm->out       */
                int dato = shm->buffer[shm->out];
                shm->out = (shm->out + 1) % N;
                sem_post(mutex);
                sem_post(vacio);

                printf("  Consumidor %d consumió: %d\n", i + 1, dato);
                usleep(80000);
            }
            printf("  Consumidor %d terminó.\n", i + 1);
            sem_close(mutex); sem_close(lleno); sem_close(vacio);
            munmap(shm, sizeof(Buffer));
            exit(0);
        }
    }

    /* ── 5. Esperar y limpiar ── */
    for (int i = 0; i < PRODUCTORES + CONSUMIDORES; i++) wait(NULL);

    sem_close(mutex);   sem_unlink(SEM_MUTEX);
    sem_close(lleno);   sem_unlink(SEM_LLENO);
    sem_close(vacio);   sem_unlink(SEM_VACIO);
    munmap(shm, sizeof(Buffer));
    shm_unlink(SHM_NOMBRE);

    printf("Total producido: %d  Total consumido: %d\n",
           PRODUCTORES * ITEMS, PRODUCTORES * ITEMS);
    return 0;
}