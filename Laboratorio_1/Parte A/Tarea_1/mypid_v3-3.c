/***************************************
 * mypid_v3-3.c
 ***************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int main(void) {
    pid_t child, father, grandfather;
    char comando[30];

    grandfather = getppid();
    if((child=fork())) {
        fprintf(stderr,"\nfork() devolvió %d => soy el padre con ID = %d\n", child,getpid());
        wait(NULL);
    }
    else {
        fprintf(stderr,"\nfork() devolvió %d => soy el hijo con ID = %d \n\n", child,getpid());
        sprintf(comando,"pstree -p %d",grandfather);
        system(comando);
        father = getppid();
    }
    return 0;
}