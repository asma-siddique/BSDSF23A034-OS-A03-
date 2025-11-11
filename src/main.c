#include "shell.h"

int main() {
    char* cmdline;
    char** arglist;

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {

        /* ---- !n re-execution feature ---- */
        if (cmdline[0] == '!') {
            int n = atoi(cmdline + 1);
            char* old_cmd = get_history_command(n);
            if (old_cmd) {
                printf("%s\n", old_cmd);
                free(cmdline);
                cmdline = old_cmd;
            } else {
                free(cmdline);
                continue;
            }
        }

        /* ---- Add command to history ---- */
        add_to_history(cmdline);

        /* ---- Tokenize & execute ---- */
        if ((arglist = tokenize(cmdline)) != NULL) {
            if (!handle_builtin(arglist))
                execute(arglist);

            for (int i = 0; arglist[i] != NULL; i++)
                free(arglist[i]);
            free(arglist);
        }

        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}

