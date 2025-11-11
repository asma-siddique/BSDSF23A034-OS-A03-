#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char* cmdline;

    printf("========================================\n");
    printf("   FCIT Shell - If-Then-Else Support\n");
    printf("========================================\n");
    printf("Type 'exit' to quit\n\n");

    while (1) {
        cmdline = read_cmd_if();
        if (cmdline == NULL) {
            continue;
        }

        if (strcmp(cmdline, "exit") == 0) {
            free(cmdline);
            break;
        }

        char** args = tokenize(cmdline);
        execute_command(args);
        
        // Free tokens
        for (int i = 0; args[i] != NULL; i++) {
            free(args[i]);
        }
        free(args);
        
        free(cmdline);
    }

    printf("Shell exited.\n");
    return 0;
}
