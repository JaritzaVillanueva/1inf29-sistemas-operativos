/* isengfork-2.c (c) 2007-2009 Rahmat M. Samik-Ibrahim, GPL-like */
/* ********************************************************** */
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    int ii = 0;
    pid_t root_pid = getpid();
    char comando[50];

    if (fork() == 0) ii++;
    waitpid(-1, NULL, 0);

    if (fork() == 0) ii++;
    waitpid(-1, NULL, 0);

    if (fork() == 0) ii++;
    waitpid(-1, NULL, 0);

    printf("Result = %3.3d \n", ii);
    fflush(stdout);

    if (getpid() == root_pid) {
        sprintf(comando, "pstree -p %d", root_pid);
        system(comando);
    }

    return 0;
}