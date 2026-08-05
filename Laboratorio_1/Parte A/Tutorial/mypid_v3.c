/**************************************
 *          mypid_v3.c
 **************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

int main(void) {
    pid_t child, father, grandfather;
    char comando[30];

    grandfather = getpid();
    if((child=fork())) {
        fprintf(stderr, "\nfork() devolvio %d => soy el padre con ID = %d\n", child, getpid());
        pause(); // Espera a que el proceso hijo termine
    }
    else {
        fprintf(stderr, "\nfork() devolvio %d => soy el hijo con ID = %d\n", child, getpid());
        sprintf(comando, "pstree -p %d", grandfather); // Comando para mostrar el árbol de procesos
        system(comando);  // Ejecuta el comando para mostrar el árbol de procesos
        father = getppid();
        kill(father, SIGTERM); // Termina el proceso padre
    }
    return 0;
}