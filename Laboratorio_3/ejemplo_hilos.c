#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_HILOS 3

void *trabajador(void *arg) {
    int id = *((int *)arg);

    printf("Hilo %d iniciado. PID: %d\n", id, getpid());

    sleep(20); // Mantiene vivo el hilo para verlo con ps/top

    printf("Hilo %d terminado.\n", id);

    return NULL;
}

int main() {
    pthread_t hilos[NUM_HILOS];
    int ids[NUM_HILOS];

    printf("Main iniciado. PID del proceso: %d\n", getpid());

    for (int i = 0; i < NUM_HILOS; i++) {
        ids[i] = i;

        if (pthread_create(&hilos[i], NULL, trabajador, &ids[i]) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    sleep(10); // Mantiene vivo también al hilo principal

    for (int i = 0; i < NUM_HILOS; i++) {
        pthread_join(hilos[i], NULL);
    }

    printf("Todos los hilos terminaron.\n");

    return 0;
}