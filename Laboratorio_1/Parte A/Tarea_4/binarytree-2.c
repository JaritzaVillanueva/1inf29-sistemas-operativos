/* binarytree-2.c (c) 2010 Alejandro T. Bello Ruiz, GPL-like */
/* ****************************************************** */
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

double final;
pid_t pid_root;

void crea_arbol(int);

int main(int narg, char *argv[]) {
    int n;

    n = atoi(argv[1]); // string a un entero
    final = pow(2, (n - 1)); // numero de nodos en el ultimo nivel del arbol
    pid_root = getpid();
    crea_arbol(1);

    return 0;
}

void crea_arbol(int x) {
    if (x >= final) {
        sleep(2); // la hoja se mantiene viva para que pstree la vea
        return;
    }

    if (!fork()) { crea_arbol(2 * x); exit(0); }   // hijo izquierdo termina solo
    if (!fork()) { crea_arbol(2 * x + 1); exit(0); } // hijo derecho termina solo

    // Solo el proceso raíz ejecuta pstree, tras construir el árbol
    if (x == 1) {
        sleep(1); // esperamos a que las hojas estén en su sleep(2)
        char comando[64];
        sprintf(comando, "pstree -p %d", pid_root);
        system(comando);
    }

    wait(NULL);
    wait(NULL);
}