#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

sem_t sala;

void *cliente(void *arg) {
    int id = *((int *)arg);

    struct timespec limite;
    clock_gettime(CLOCK_REALTIME, &limite);
    limite.tv_sec += 3; // Espera máximo 3 segundos

    printf("Cliente %d: intentando entrar...\n", id);

    if (sem_timedwait(&sala, &limite) == -1) {
        if (errno == ETIMEDOUT) {
            printf("Cliente %d: esperé demasiado, me retiro.\n", id);
            return NULL;
        }
    }

    printf("Cliente %d: entré a la sala.\n", id);

    sleep(1); // Ocupa el asiento 5 segundos

    printf("Cliente %d: salgo de la sala.\n", id);

    sem_post(&sala);

    return NULL;
}

int main() {
    sem_init(&sala, 0, 1);

    pthread_t h1, h2;
    int id1 = 1, id2 = 2;

    pthread_create(&h1, NULL, cliente, &id1);
    sleep(1);
    pthread_create(&h2, NULL, cliente, &id2);

    pthread_join(h1, NULL);
    pthread_join(h2, NULL);

    sem_destroy(&sala);

    return 0;
}