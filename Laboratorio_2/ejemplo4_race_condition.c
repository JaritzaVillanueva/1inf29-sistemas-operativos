/* ejemplo4_race_condition.c
 * Demuestra una condición de carrera con memoria compartida.
 * Compilar: gcc ejemplo4_race_condition.c -o ejemplo4 -lrt
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#define SHM_NOMBRE "/ejemplo4_shm"
#define ITERACIONES 10
int main()
{
    int fd = shm_open(SHM_NOMBRE, O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(int));
    int *contador = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                         MAP_SHARED, fd, 0);
    close(fd);
    *contador = 0;
    pid_t pid = fork();
    if (pid == 0)
    {
        /* HIJO: incrementa el contador 100,000 veces. */
        for (int i = 0; i < ITERACIONES; i++)
        {
            (*contador)++;
        }
        printf("Hijo terminó. Valor actual: %d\n", *contador);
        munmap(contador, sizeof(int));
        exit(0);
    }
    /* PADRE: también incrementa el contador 100,000 veces. */
    for (int i = 0; i < ITERACIONES; i++)
    {
        (*contador)++;
    }
    printf("Padre terminó. Valor actual: %d\n", *contador);
    wait(NULL);
    printf("Valor final del contador: %d\n", *contador);
    printf("Valor esperado: %d\n", 2 * ITERACIONES);
    munmap(contador, sizeof(int));
    shm_unlink(SHM_NOMBRE);
    return 0;
}