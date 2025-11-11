#include "shell.h"

char* history[HISTORY_SIZE];
int history_count = 0;

/* ---------------- Read command line ---------------- */
char* read_cmd(char* prompt, FILE* fp) {
    printf("%s", prompt);
    char* cmdline = malloc(sizeof(char) * MAX_LEN);
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
        return NULL;  // Handle Ctrl+D
    }

    cmdline[pos] = '\0';
    return cmdline;
}

/* ---------------- Tokenize input ---------------- */
char** tokenize(char* cmdline) {
    if (!cmdline || cmdline[0] == '\0') return NULL;

    char** arglist = malloc(sizeof(char*) * (MAXARGS + 1));
    if (!arglist) {
        perror("malloc failed");
        exit(1);
    }

    for (int i = 0; i < MAXARGS + 1; i++) {
        arglist[i] = malloc(sizeof(char) * ARGLEN);
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
        while (*cp == ' ' || *cp == '\t') cp++;
        if (*cp == '\0') break;

        start = cp;
        len = 1;
        while (*++cp != '\0' && !(*cp == ' ' || *cp == '\t')) len++;

        strncpy(arglist[argnum], start, len);
        arglist[argnum][len] = '\0';
        argnum++;
    }

    if (argnum == 0) {
        for (int i = 0; i < MAXARGS + 1; i++) free(arglist[i]);
        free(arglist);
        return NULL;
    }

    arglist[argnum] = NULL;
    return arglist;
}

/* ---------------- History management ---------------- */
void add_to_history(const char* cmd) {
    if (!cmd || cmd[0] == '\0') return;

    int index = history_count % HISTORY_SIZE;
    if (history[index]) free(history[index]);
    history[index] = strdup(cmd);
    history_count++;
}

void show_history(void) {
    int start = (history_count > HISTORY_SIZE) ? history_count - HISTORY_SIZE : 0;
    int end = history_count;

    for (int i = start; i < end; i++) {
        int index = i % HISTORY_SIZE;
        printf("%d %s\n", i + 1, history[index]);
    }
}

char* get_history_command(int n) {
    if (n < 1 || n > history_count || (history_count - n) >= HISTORY_SIZE) {
        printf("Error: No such command in history.\n");
        return NULL;
    }
    int index = (n - 1) % HISTORY_SIZE;
    return strdup(history[index]);
}

