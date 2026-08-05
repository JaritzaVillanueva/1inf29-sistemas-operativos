/***************************************
 * chainp.c
 ***************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* 
 * Este programa crea una cadena de procesos. Es decir el padre
 * crea un hijo, este a su vez crea otro y asi en forma sucesiva
 * Ejm 2.5 del libro UNIX Programacion Practica - Kay Robbins
 *                                                Steve Robbins
 * Modificado por 20213699 - Informatica PUCP
*/

int main(void) {
    int i, status;
    pid_t child; // pid_t es un tipo definido en types.h

    fprintf(stderr,"Soy el padre = %d\n", getpid());

    for(i = 0; i<4; ++i) if ((child=fork())) break;
    fprintf(stderr, "Esta es la vuelta Nro %d\n", i);
    fprintf(stderr, "Recibi de fork el valor de %d\n", child);
    fprintf(stderr, "Soy el proceso %d con padre %d\n\n", getpid(), getppid());
    wait(&status);
    return 0;
}