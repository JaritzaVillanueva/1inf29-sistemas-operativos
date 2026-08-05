/* ejemplo1_shm_basico.c
 * Demuestra el ciclo de vida de un segmento de memoria compartida POSIX.
 * Compilar: gcc ejemplo1_shm_basico.c -o ejemplo1 -lrt
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#define SHM_NOMBRE "/ejemplo1_shm"
#define SHM_TAMANIO sizeof(int)
int main()
{
    /* Paso 1: Crear el segmento.
     * O_CREAT: crea si no existe.
     * O_RDWR: abre para lectura y escritura.
     * 0666: permisos de lectura/escritura para todos.
     * Retorna un file descriptor, similar a open(). */
    int fd = shm_open(SHM_NOMBRE, O_CREAT | O_RDWR, 0666);
    if (fd == -1)
    {
        perror("shm_open falló");
        exit(EXIT_FAILURE);
    }
    printf("Segmento creado. File descriptor: %d\n", fd);
    /* Paso 2: Establecer el tamaño.
     * Sin este paso, el segmento tiene tamaño 0 y mmap fallará.
     * Usamos sizeof(int) porque solo almacenaremos un entero. */
    if (ftruncate(fd, SHM_TAMANIO) == -1)
    {
        perror("ftruncate falló");
        exit(EXIT_FAILURE);
    }
    printf("Tamaño establecido: %lu bytes\n", (unsigned long)SHM_TAMANIO);
    /* Paso 3: Mapear el segmento a un puntero.
     * NULL: el kernel elige la dirección virtual.
     * PROT_READ | PROT_WRITE: podemos leer y escribir.
     * MAP_SHARED: los cambios son visibles para otros procesos.
     * fd: el file descriptor del segmento.
     * 0: offset desde el inicio del segmento. */
    int *ptr = mmap(NULL, SHM_TAMANIO, PROT_READ | PROT_WRITE,
                    MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED)
    {
        perror("mmap falló");
        exit(EXIT_FAILURE);
    }
    /* Paso 4: El fd ya no se necesita después de mmap.
     * El mapeo en memoria persiste independientemente del fd. */
    close(fd);
    /* Paso 5: Usar el segmento como un puntero normal. */
    *ptr = 2026;
    printf("Valor escrito: %d\n", *ptr);
    printf("Dirección virtual del mapeo: %p\n", (void *)ptr);
    /* Paso 6: Desmapear y eliminar. */
    munmap(ptr, SHM_TAMANIO);
    shm_unlink(SHM_NOMBRE);
    printf("Segmento desmapeado y eliminado.\n");
    return 0;
}