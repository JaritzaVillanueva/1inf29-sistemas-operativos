/***************************************
 * fanp-2.c
 ***************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

/* 
 * Este programa crea un abanico de procesos. Es decir el padre crea
 * tres hijos
 * Ejm 2.6 del libro UNIX Programacion Practica - Kay Robbins
 *                                                Steve Robbins
 * Modificado por 20213699 - Informatica PUCP
*/

int main(void) {
    int i, status;
    pid_t child, pid_padre;
    char comando[50];

    pid_padre = getpid();
    for(i = 0; i<4; ++i)
        if((child=fork()) <= 0) break;
        else fprintf(stderr, "Vuelta nro %d .... creando el proceso %d\n", i, child);
    if(i == 4) {
        sprintf(comando,"pstree -p %d",pid_padre);
        system(comando);
    }
    if (pid_padre == getpid()) for (i=0; i<4; ++i) wait(&status); // el padre espera a sus hijos a cada  uno
    fprintf(stderr, "Este es el proceso %d con padre %d\n", getpid(), getppid());
    return 0;
}