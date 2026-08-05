#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
int main(void) {
    int valor = 0;
    pid_t pid = fork();
    if (pid == 0) {
        /* HIJO */
        valor = 111;
        sleep(1);   /* Para que la salida no se mezcle */
        printf("[Hijo]  PID=%d  direccion=%p  valor=%d\n",
               getpid(), (void *)&valor, valor);
    } else {
        /* PADRE */
        valor = 999;
        printf("[Padre] PID=%d  direccion=%p  valor=%d\n",
               getpid(), (void *)&valor, valor);
        wait(NULL);
    }
    return 0;
}