#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char* handle_history_recall(char *cmdline) {
    if (cmdline[0] != '!') return strdup(cmdline);

    int n = atoi(cmdline + 1);
    char *hist_cmd = get_history_command(n);
    if (!hist_cmd) {
        printf("No such command in history: !%d\n", n);
        return NULL;
    }
    printf("Re-executing: %s\n", hist_cmd);
    return strdup(hist_cmd);
}

int main() {
    char *cmdline;

    init_history();
    init_jobs();

    printf("========================================\n");
    printf("  FCIT Shell - Feature 6 & 8\n");
    printf("========================================\n");
    printf("Type 'exit' to quit\n\n");

    while (1) {
        cleanup_zombies();
        printf("FCIT> ");
        fflush(stdout);

        cmdline = malloc(INPUT_BUFFER_SIZE);
        if (!fgets(cmdline, INPUT_BUFFER_SIZE, stdin)) {
            printf("\n");
            break;
        }

        // Remove newline
        cmdline[strcspn(cmdline, "\n")] = 0;
        trim(cmdline);
        if (strlen(cmdline) == 0) {
            free(cmdline);
            continue;
        }

        // History recall !n
        char *expanded_cmd = handle_history_recall(cmdline);
        free(cmdline);
        if (!expanded_cmd) continue;

        add_to_history(expanded_cmd);

        // Split by ';' for command chaining
        char *saveptr1;
        char *command_group = strtok_r(expanded_cmd, ";", &saveptr1);
        while (command_group != NULL) {
            trim(command_group);
            if (strlen(command_group) == 0) {
                command_group = strtok_r(NULL, ";", &saveptr1);
                continue;
            }

            // Background check '&'
            int background = 0;
            size_t len = strlen(command_group);
            if (len > 0 && command_group[len - 1] == '&') {
                background = 1;
                command_group[len - 1] = '\0';
                trim(command_group);
            }

            execute_command(command_group, background);

            command_group = strtok_r(NULL, ";", &saveptr1);
        }

        free(expanded_cmd);
    }

    printf("Shell exited.\n");
    return 0;
}

