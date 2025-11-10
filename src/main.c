#include "shell.h"

int main() {
    char* cmdline;
    char** arglist;

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {
        arglist = tokenize(cmdline);
        if (arglist != NULL) {

            // Check for built-in first
            if (!handle_builtin(arglist)) {
                execute(arglist); // Only run external if not a built-in
            }

            // Free memory
            for (int i = 0; arglist[i] != NULL; i++) free(arglist[i]);
            free(arglist);
        }
        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}

