/* ejemplo2_sem_init.c
 * Sincronización entre dos hilos con un semáforo sin nombre.
 * Un hilo productor prepara un dato y un hilo consumidor espera hasta que esté listo.
 *
 * Compilar:
 * gcc ejemplo2_sem_init.c -o ejemplo2 -lpthread
 */

#include <stdio.h>      // printf(), perror()
#include <stdlib.h>     // exit(), EXIT_FAILURE
#include <pthread.h>    // pthread_create(), pthread_join()
#include <semaphore.h>  // sem_t, sem_init(), sem_wait(), sem_post(), sem_destroy()
#include <unistd.h>     // sleep()

// Semáforo global compartido por los hilos
sem_t listo;

// Variable global compartida por productor y consumidor
int dato = 0;

// Función que ejecutará el hilo productor
void *productor(void *arg) {
    // Evita warning porque no usamos el argumento
    (void)arg;

    printf("Productor: preparando dato...\n");

    // Simula que producir el dato toma tiempo
    sleep(2);

    // El productor escribe en la variable compartida
    dato = 42;

    printf("Productor: dato = %d, notificando.\n", dato);

    // Incrementa el semáforo y despierta al consumidor si estaba bloqueado
    sem_post(&listo);

    return NULL;
}

// Función que ejecutará el hilo consumidor
void *consumidor(void *arg) {
    // Evita warning porque no usamos el argumento
    (void)arg;

    printf("Consumidor: esperando dato...\n");

    // Espera hasta que el productor haga sem_post()
    // Como el semáforo inicia en 0, aquí se bloquea
    sem_wait(&listo);

    // Cuando despierta, el dato ya fue producido
    printf("Consumidor: dato recibido = %d\n", dato);

    return NULL;
}

int main() {
    // Inicializa el semáforo sin nombre
    // &listo: dirección del semáforo
    // 0: se compartirá entre hilos del mismo proceso
    // 0: valor inicial; el primer sem_wait() se bloqueará
    if (sem_init(&listo, 0, 0) != 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    // Identificadores de los hilos
    pthread_t h_prod, h_cons;

    // Crea el hilo productor
    pthread_create(&h_prod, NULL, productor, NULL);

    // Crea el hilo consumidor
    pthread_create(&h_cons, NULL, consumidor, NULL);

    // Main espera a que termine el productor
    pthread_join(h_prod, NULL);

    // Main espera a que termine el consumidor
    pthread_join(h_cons, NULL);

    // Libera los recursos del semáforo
    sem_destroy(&listo);

    return 0;
}