/* forknpipe.c (c) 2007-2009 Rahmat M. Samik-Ibrahim, GPL-like */
/* ******************************************************* */
#define BUFSIZE 64 // tamaño del buffer de lectura del padre
#define WLOOP   5 // numero de veces que un hijo escribe en el pipe
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

int main(void) {
    char buffer[BUFSIZE];
    char message[]="Hello, what's up?\n";
    int ii, pipe_fd[2];
    
    pipe(pipe_fd); // se crea el pipe, pipe_fd[0] es para lectura, pipe_fd[1] es para escritura
    
    if (fork() == 0) {       /* child ***************************/
        /* el hijo cierra el extremo de lectura del pipe */
        close(pipe_fd[0]); 
        printf("I am PID [%d] (child).\n", (int) getpid());
        /* el hijo escribe en el pipe WLOOP veces */
        for (ii=0; ii<WLOOP; ii++)
            write(pipe_fd[1], message, sizeof(message)-1);
        /* el hijo cierra el extremo de escritura del pipe */
        close(pipe_fd[1]);
    } else {                 /* parent **************************/
        /* el padre cierra el extremo de escritura del pipe */
        close(pipe_fd[1]);
        printf("I am PID [%d] (parent).\n", (int) getpid());
        memset(buffer, 0, sizeof(buffer)); // se limpia el buffer
        /* el padre lee del pipe hasta que no haya más datos */
        while ((ii=read(pipe_fd[0], buffer, BUFSIZE-1)) != 0) {
            printf("PARENT READ [%d]:\n%s\n", (int) ii, buffer);
            memset(buffer, 0, sizeof(buffer)); // se limpia el buffer para la siguiente lectura
        }
        /* el padre cierra el extremo de lectura del pipe */
        close(pipe_fd[0]);
    }
    return 0;
}