/***************************************
 * chainp-2.c
 ***************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

/* 
 * Este programa crea una cadena de procesos. Es decir el padre
 * crea un hijo, este a su vez crea otro y asi en forma sucesiva
 * Ejm 2.5 del libro UNIX Programacion Practica - Kay Robbins
 *                                                Steve Robbins
 * Modificado por 20213699 - Informatica PUCP
*/

int main(void) {
    int i, status;
    pid_t child, grandfather; // pid_t es un tipo definido en types.h
    char comando[50];

    grandfather=getpid();
    fprintf(stderr,"Soy el padre = %d\n", getpid());

    for(i = 0; i<4; ++i) if ((child=fork())) break;
    fprintf(stderr, "Esta es la vuelta Nro %d\n", i);
    fprintf(stderr, "Recibi de fork el valor de %d\n", child);
    fprintf(stderr, "Soy el proceso %d con padre %d\n\n", getpid(), getppid());

    if(i == 4){
        sprintf(comando,"pstree -p %d",grandfather);
        system(comando);
    }
    if(child > 0) wait(&status); // cada padre espera a su hijo, el hijo a su vez espera a su hijo y asi sucesivamente
    return 0;
}