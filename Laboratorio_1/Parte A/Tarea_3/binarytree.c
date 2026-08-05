/* binarytree.c (c) 2010 Alejandro T. Bello Ruiz, GPL-like */
/* ****************************************************** */
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

double final;

void crea_arbol(int);

int main(int narg, char *argv[]) {
    int n;

    n = atoi(argv[1]); // string a un entero
    final = pow(2, (n - 1)); // numero de nodos en el ultimo nivel del arbol
    crea_arbol(1);
    return 0;
}

void crea_arbol(int x) {
    if (x >= final) return;
    /* el hijo entra en el if, el padre no porque tiene PID positivo */
        /* El hijo:
        1. construye recursivamente su subárbol izquierdo
        2. se queda detenido con pause() */
    if (!fork()) { crea_arbol(2 * x); pause(); } // hijo izquierdo
    if (!fork()) { crea_arbol(2 * x + 1); pause(); } // hijo derecho
    
    // el padre espera a que terminen sus hijos
    // NOTA: el padre queda bloqueado en wait()
    wait(NULL);
    wait(NULL);
}