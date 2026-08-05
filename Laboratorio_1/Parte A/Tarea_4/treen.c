#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    int n = atoi(argv[1]);
    pid_t pid_root = getpid();
    int i;

    /* el padre crea n hijos (nivel 1)*/
    for (i =0; i< n; i++){
        if(fork()==0){
            /*cada hijo crea n hijos (nivel 2 = hojas)*/
            int j;
            for(j=0; j<n; j++){
                if(fork()==0){
                    // Soy hoja: me quedo vivo para que pstree me vea
                    sleep(2);
                    exit(0);
                }
            }
            /* el hijo del nivel 1 espera a sus hijos*/
            for (j=0; j<n; j++){
                wait(NULL);
            }
            exit(0);
        }
    }

    /*solo el padre (raiz) llega aqui*/
    /*Esperemos a que todos los nietos esten en su sleep*/
    sleep(1);

    char comando[100];
    sprintf(comando,"pstree -p %d",pid_root);
    system(comando);

    /*Recogemos los n hijos directos*/
    for(i=0; i<n; i++){
        wait(NULL);
    }
    return 0;
}