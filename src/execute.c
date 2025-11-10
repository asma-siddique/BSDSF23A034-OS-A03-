#include "shell.h"

// Handles built-in commands: returns 1 if handled, 0 if not
int handle_builtin(char** arglist) {
    if (!arglist || !arglist[0]) return 0;

    if (strcmp(arglist[0], "cd") == 0) {
        if (!arglist[1]) {
            fprintf(stderr, "cd: missing argument\n");
        } else if (chdir(arglist[1]) != 0) {
            perror("cd failed");
        }
        return 1;
    }

    if (strcmp(arglist[0], "exit") == 0) {
        exit(0);
    }

    if (strcmp(arglist[0], "help") == 0) {
        printf("Built-in commands:\n");
        printf("cd <directory> - Change directory\n");
        printf("exit - Exit the shell\n");
        printf("help - Show this help message\n");
        printf("jobs - Show job status (not implemented)\n");
        return 1;
    }

    if (strcmp(arglist[0], "jobs") == 0) {
        printf("Job control not yet implemented.\n");
        return 1;
    }

    return 0; // Not a built-in
}

// Executes external commands (fork + exec)
int execute(char** arglist) {
    if (!arglist || !arglist[0]) return 0;

    int status;
    int cpid = fork();

    if (cpid < 0) {
        perror("fork failed");
        exit(1);
    }

    if (cpid == 0) { // Child process
        execvp(arglist[0], arglist);
        perror("Command not found");
        exit(1);
    } else { // Parent process
        waitpid(cpid, &status, 0);
    }

    return 0;
}

