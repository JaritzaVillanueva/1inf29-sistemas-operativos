/* ejemplo3_trywait.c
 * Demuestra la diferencia entre sem_wait y sem_trywait.
 * Modela una sala con 3 asientos.
 * Si la sala está llena, los clientes no esperan: se retiran.
 *
 * Compilar:
 * gcc ejemplo3_trywait.c -o ejemplo3 -lpthread
 */

#include <stdio.h>      // printf(), perror()
#include <stdlib.h>     // funciones generales de C
#include <pthread.h>    // pthread_create(), pthread_join()
#include <semaphore.h>  // sem_t, sem_init(), sem_trywait(), sem_post(), sem_destroy()
#include <unistd.h>     // sleep()
#include <errno.h>      // errno, EAGAIN

#define ASIENTOS 3       // Capacidad máxima de la sala
#define NUM_CLIENTES 6   // Número total de clientes/hilos

// Semáforo que representa los asientos disponibles
sem_t sala;

// Función que ejecutará cada hilo cliente
void *cliente(void *arg) {
    // Recuperamos el ID del cliente
    int id = *((int *)arg);

    // sem_trywait intenta tomar un asiento
    // Si hay asiento disponible, decrementa el semáforo y retorna 0
    // Si no hay asiento, retorna -1 y no bloquea al hilo
    if (sem_trywait(&sala) == -1) {

        // EAGAIN significa que el recurso no estaba disponible
        if (errno == EAGAIN) {
            printf("Cliente %d: sala llena, me retiro.\n", id);
            return NULL;
        }

        // Si ocurrió otro error distinto, lo mostramos
        perror("sem_trywait");
        return NULL;
    }

    // Si llegó aquí, consiguió un asiento
    printf("Cliente %d: tomé asiento.\n", id);

    // Simula el tiempo que el cliente permanece en la sala
    sleep(2);

    // El cliente libera el asiento
    printf("Cliente %d: libero el asiento.\n", id);

    // Incrementa el semáforo: hay un asiento disponible otra vez
    sem_post(&sala);

    return NULL;
}

int main() {
    // Inicializa el semáforo con 3 asientos disponibles
    // 0 indica que se comparte entre hilos del mismo proceso
    sem_init(&sala, 0, ASIENTOS);

    // Arreglo de hilos
    pthread_t hilos[NUM_CLIENTES];

    // Arreglo de IDs para cada cliente
    int ids[NUM_CLIENTES];

    // Creamos 6 clientes casi al mismo tiempo
    for (int i = 0; i < NUM_CLIENTES; i++) {
        ids[i] = i;

        pthread_create(&hilos[i], NULL, cliente, &ids[i]);
    }

    // Esperamos a que todos los clientes terminen
    for (int i = 0; i < NUM_CLIENTES; i++) {
        pthread_join(hilos[i], NULL);
    }

    // Liberamos los recursos del semáforo
    sem_destroy(&sala);

    return 0;
}