/* ejemplo3_semaforo.c
 * Introducción a semáforos POSIX con nombre.
 * Compilar: gcc ejemplo3_semaforo.c -o ejemplo3 -lrt -lpthread
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <unistd.h>
#define SEM_NOMBRE "/ejemplo3_sem"
int main()
{
    /* Crear un semáforo con nombre.
     * O_CREAT: crea si no existe.
     * 0666: permisos.
     * 0: valor inicial (bloqueado).
     * Un semáforo en 0 significa: quien haga sem_wait() se bloqueará. */
    sem_t *sem = sem_open(SEM_NOMBRE, O_CREAT, 0666, 0);
    if (sem == SEM_FAILED)
    {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    pid_t pid = fork();
    if (pid == 0)
    {
        /* PROCESO HIJO */
        printf("Hijo [PID=%d]: esperando señal del padre...\n", getpid());
        /* sem_wait decrementa el semáforo.
         * Si el valor es 0, el proceso se BLOQUEA hasta que alguien
         * haga sem_post (incremento). */
        sem_wait(sem);
        printf("Hijo [PID=%d]: señal recibida, continuando.\n", getpid());
        sem_close(sem);
        exit(0);
    }
    /* PROCESO PADRE */
    printf("Padre [PID=%d]: realizando trabajo previo...\n", getpid());
    sleep(2); /* Simula trabajo que toma 2 segundos. */
    printf("Padre [PID=%d]: enviando señal al hijo.\n", getpid());
    /* sem_post incrementa el semáforo.
     * Si hay un proceso bloqueado en sem_wait, lo despierta. */
    //sem_post(sem);
    wait(NULL);
    /* Limpieza: cerrar y eliminar el semáforo. */
    sem_close(sem);
    sem_unlink(SEM_NOMBRE);
    printf("Padre [PID=%d]: semáforo eliminado.\n", getpid());
    return 0;
}