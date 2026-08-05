/* ejemplo1_pthreads.c
 * Este programa crea 3 hilos usando pthreads.
 * Cada hilo recibe un ID, imprime mensajes y termina.
 * Compilar:
 * gcc ejemplo1_pthreads.c -o ejemplo1 -lpthread
 */

#include <stdio.h>      // Permite usar printf()
#include <stdlib.h>     // Permite usar exit() y EXIT_FAILURE
#include <pthread.h>    // Permite usar pthread_create() y pthread_join()
#include <unistd.h>     // Permite usar sleep()

#define NUM_HILOS 3     // Cantidad de hilos que se crearán

// Función que ejecutará cada hilo
void *trabajador(void *arg) {
    // Convertimos el argumento void* a int*
    // Luego obtenemos el valor del ID
    int id = *((int *)arg);

    // Mensaje cuando el hilo inicia
    printf("Hilo %d: comencé a trabajar.\n", id);

    // Simula que el hilo está trabajando durante 1 segundo
    sleep(1);

    // Mensaje cuando el hilo termina
    printf("Hilo %d: terminé.\n", id);

    // El hilo termina sin devolver información
    return NULL;
}

int main() {
    // Arreglo donde se guardan los identificadores de los hilos
    pthread_t hilos[NUM_HILOS];

    // Arreglo con los IDs que recibirá cada hilo
    int ids[NUM_HILOS];

    // Crear los hilos
    for (int i = 0; i < NUM_HILOS; i++) {
        // Guardamos un ID distinto para cada hilo
        ids[i] = i;

        // Creamos un hilo
        if (pthread_create(&hilos[i], NULL, trabajador, &ids[i]) != 0) {
            // Si pthread_create falla, mostramos el error
            perror("pthread_create");

            // Terminamos el programa con error
            exit(EXIT_FAILURE);
        }
    }

    // Esperar a que todos los hilos terminen
    for (int i = 0; i < NUM_HILOS; i++) {
        pthread_join(hilos[i], NULL);
    }

    // Este mensaje aparece solo cuando todos los hilos ya terminaron
    printf("Main: todos los hilos terminaron.\n");

    return 0;
}