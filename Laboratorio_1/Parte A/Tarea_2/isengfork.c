/* isengfork.c (c) 2007-2009 Rahmat M. Samik-Ibrahim, GPL-like */
/* ********************************************************** */
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    int ii = 0;

    if (fork() == 0) ii++;
    waitpid(-1, NULL, 0);

    if (fork() == 0) ii++;
    waitpid(-1, NULL, 0);

    if (fork() == 0) ii++;
    waitpid(-1, NULL, 0);

    printf("Result = %3.3d \n", ii);

    return 0;
}