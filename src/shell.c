#include "shell.h"

// Read a line of input from the user
char* read_cmd(char* prompt, FILE* fp) {
    printf("%s", prompt);
    char* cmdline = (char*) malloc(sizeof(char) * MAX_LEN);
    if (!cmdline) {
        perror("malloc failed");
        exit(1);
    }

    int c, pos = 0;
    while ((c = getc(fp)) != EOF) {
        if (c == '\n') break;
        cmdline[pos++] = c;
    }

    if (c == EOF && pos == 0) {
        free(cmdline);
        return NULL; // Handle Ctrl+D
    }

    cmdline[pos] = '\0';
    return cmdline;
}

// Tokenize the command line into arguments
char** tokenize(char* cmdline) {
    if (!cmdline || cmdline[0] == '\0') return NULL;

    char** arglist = (char**) malloc(sizeof(char*) * (MAXARGS + 1));
    if (!arglist) {
        perror("malloc failed");
        exit(1);
    }

    for (int i = 0; i < MAXARGS + 1; i++) {
        arglist[i] = (char*) malloc(sizeof(char) * ARGLEN);
        if (!arglist[i]) {
            perror("malloc failed");
            exit(1);
        }
        bzero(arglist[i], ARGLEN);
    }

    char* cp = cmdline;
    char* start;
    int len;
    int argnum = 0;

    while (*cp != '\0' && argnum < MAXARGS) {
        while (*cp == ' ' || *cp == '\t') cp++; // Skip whitespace
        if (*cp == '\0') break;

        start = cp;
        len = 1;
        while (*++cp != '\0' && !(*cp == ' ' || *cp == '\t')) len++;

        strncpy(arglist[argnum], start, len);
        arglist[argnum][len] = '\0';
        argnum++;
    }

    if (argnum == 0) { // No arguments
        for (int i = 0; i < MAXARGS + 1; i++) free(arglist[i]);
        free(arglist);
        return NULL;
    }

    arglist[argnum] = NULL; // Null-terminate argument list
    return arglist;
}

