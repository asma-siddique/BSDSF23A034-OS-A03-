#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

void execute(char** arglist) {
    if (arglist[0] == NULL) return;

    pid_t pid = fork();
    
    if (pid == 0) {
        execvp(arglist[0], arglist);
        fprintf(stderr, "Command not found: %s\n", arglist[0]);
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("fork failed");
    }
}
