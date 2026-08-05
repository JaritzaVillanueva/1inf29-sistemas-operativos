/* ejemplo2_shm_fork.c
 * Dos procesos comparten un entero mediante memoria compartida POSIX.
 * Compilar: gcc ejemplo2_shm_fork.c -o ejemplo2 -lrt
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#define SHM_NOMBRE "/ejemplo2_shm"
#define SHM_TAMANIO sizeof(int)

int main()
{
    /* Crear y mapear el segmento ANTES del fork.
     * Así ambos procesos heredan el mismo mapeo. */
    int fd = shm_open(SHM_NOMBRE, O_CREAT | O_RDWR, 0666);
    if (fd == -1)
    {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    ftruncate(fd, SHM_TAMANIO);
    int *ptr = mmap(NULL, SHM_TAMANIO, PROT_READ | PROT_WRITE,
                    MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    close(fd);
    /* Inicializar el valor a 0 antes de fork. */
    *ptr = 0;
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0)
    {
        /* PROCESO HIJO */
        printf("Hijo [PID=%d]: valor leído = %d\n", getpid(), *ptr);
        /* El hijo también puede escribir en el segmento. */
        *ptr = 100;
        printf("Hijo [PID=%d]: valor escrito = %d\n", getpid(), *ptr);
        munmap(ptr, SHM_TAMANIO);
        shm_unlink(SHM_NOMBRE);
        exit(0);
    }
    /* PROCESO PADRE */
    *ptr = 42;
    printf("Padre [PID=%d]: valor escrito = %d\n", getpid(), *ptr);
    /* Esperar a que el hijo termine. */
    wait(NULL);
    /* Leer el valor después de que el hijo terminó. */
    printf("Padre [PID=%d]: valor final = %d\n", getpid(), *ptr);
    /* Solo el padre limpia los recursos. */
    munmap(ptr, SHM_TAMANIO);
    
    return 0;
}