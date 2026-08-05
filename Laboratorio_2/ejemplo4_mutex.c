#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <semaphore.h> // Librería para semáforos

#define SHM_NOMBRE "/ejemplo4_shm"
#define SEM_NOMBRE "/ejemplo4_sem"
#define ITERACIONES 10000 // Subimos las iteraciones para probar la eficacia

int main()
{
    // 1. Crear Memoria Compartida
    int fd = shm_open(SHM_NOMBRE, O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(int));
    int *contador = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    *contador = 0;

    // 2. Crear Semáforo como MUTEX (Valor inicial = 1)
    // El valor 1 significa que el recurso está libre al inicio.
    sem_t *mutex = sem_open(SEM_NOMBRE, O_CREAT, 0666, 1);
    if (mutex == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid == 0) {
        /* HIJO */
        for (int i = 0; i < ITERACIONES; i++) {
            sem_wait(mutex);   // <--- ENTRADA: Cierra el candado
            (*contador)++;     // SECCIÓN CRÍTICA
            sem_post(mutex);   // <--- SALIDA: Abre el candado
        }
        munmap(contador, sizeof(int));
        sem_close(mutex);
        exit(0);
    }

    /* PADRE */
    for (int i = 0; i < ITERACIONES; i++) {
        sem_wait(mutex);       // <--- ENTRADA: Cierra el candado
        (*contador)++;         // SECCIÓN CRÍTICA
        sem_post(mutex);       // <--- SALIDA: Abre el candado
    }

    wait(NULL);

    printf("Valor final del contador: %d\n", *contador);
    printf("Valor esperado: %d\n", 2 * ITERACIONES);

    // 3. Limpieza
    munmap(contador, sizeof(int));
    shm_unlink(SHM_NOMBRE);
    sem_close(mutex);
    sem_unlink(SEM_NOMBRE); // Eliminar el semáforo del sistema

    return 0;
}