#include "shell.h"

// Built-in check
int is_builtin(char** arglist) {
    if (!arglist || !arglist[0]) return 0;
    return strcmp(arglist[0], "cd") == 0 ||
           strcmp(arglist[0], "exit") == 0 ||
           strcmp(arglist[0], "help") == 0 ||
           strcmp(arglist[0], "jobs") == 0;
}

// Execute built-in commands
int execute_builtin(char** arglist) {
    if (strcmp(arglist[0], "cd") == 0) {
        if (!arglist[1]) fprintf(stderr, "cd: missing argument\n");
        else if (chdir(arglist[1]) != 0) perror("cd failed");
        return 0;
    }
    if (strcmp(arglist[0], "exit") == 0) exit(0);
    if (strcmp(arglist[0], "help") == 0) {
        printf("Built-ins: cd, exit, help, jobs\n");
        return 0;
    }
    if (strcmp(arglist[0], "jobs") == 0) {
        printf("Job control not yet implemented.\n");
        return 0;
    }
    return 1;
}

// Execute any command
int execute(char** arglist) {
    if (!arglist || !arglist[0]) return 0;

    if (is_builtin(arglist)) return execute_builtin(arglist);

    int status;
    int cpid = fork();
    if (cpid < 0) { perror("fork failed"); exit(1); }
    if (cpid == 0) { // child
        execvp(arglist[0], arglist);
        perror("Command not found");
        exit(1);
    }
    waitpid(cpid, &status, 0);
    return 0;
}

