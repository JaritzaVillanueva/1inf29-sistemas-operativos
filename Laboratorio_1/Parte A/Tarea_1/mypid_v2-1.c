/***************************************
 * mypid_v2-1.c
 ***************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int main(void) {
    pid_t child;
    if((child=fork())) {
        fprintf(stderr,"\nfork() devolvió %d => soy el padre con ID = %d\n", child,getpid());
        pause();
    }
    else {
        fprintf(stderr,"\nfork() devolvió %d => soy el hijo con ID = %d \n", child,getpid());
        
    }
    return 0;
}