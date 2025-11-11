#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Handle !n recall from history
static char* handle_history_recall(char *cmdline) {
    if (cmdline[0] != '!') 
        return strdup(cmdline);  // make a copy if normal

    int n = atoi(cmdline + 1);
    char *hist_cmd = get_history_command(n);
    if (hist_cmd == NULL) {
        printf("No such command in history: !%d\n", n);
        return NULL;
    }

    printf("Re-executing: %s\n", hist_cmd);
    return strdup(hist_cmd); // duplicate safely
}

// Trim leading/trailing spaces
static void trim(char *str) {
    char *end;

    // Trim leading space
    while(isspace((unsigned char)*str)) str++;

    if(*str == 0) return; // all spaces?

    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator
    *(end+1) = '\0';
}

int main() {
    char *cmdline;

    // Initialize history and background jobs
    init_history();
    init_jobs();

    printf("========================================\n");
    printf("  FCIT Shell - Feature 6 (Jobs & History)\n");
    printf("========================================\n");
    printf("Type 'exit' to quit\n\n");

    while (1) {
        cleanup_zombies();
        printf(SHELL_PROMPT);
        fflush(stdout);

        cmdline = read_cmd();  // Read user input
        if (!cmdline) break;   // Ctrl+D or EOF

        // Handle !n history recall
        char *expanded_cmd = handle_history_recall(cmdline);
        free(cmdline);
        if (!expanded_cmd) continue;

        // Add to history
        add_to_history(expanded_cmd);

        // Handle command chaining (;)
        char *saveptr1;
        char *command_group = strtok_r(expanded_cmd, ";", &saveptr1);
        while (command_group != NULL) {
            trim(command_group);
            if (strlen(command_group) == 0) {
                command_group = strtok_r(NULL, ";", &saveptr1);
                continue;
            }

            // Handle background execution (&)
            int background = 0;
            size_t len = strlen(command_group);
            if (len > 0 && command_group[len - 1] == '&') {
                background = 1;
                command_group[len - 1] = '\0';
                trim(command_group);
            }

            // Built-in commands
            if (strcmp(command_group, "history") == 0) {
                show_history();
            } else if (strcmp(command_group, "jobs") == 0) {
                show_jobs();
            } else if (strcmp(command_group, "exit") == 0) {
                free(expanded_cmd);
                printf("Shell exited.\n");
                return 0;
            } else {
                // Execute normal command
                execute_command(command_group, background);
            }

            command_group = strtok_r(NULL, ";", &saveptr1);
        }

        free(expanded_cmd);
    }

    printf("Shell exited.\n");
    return 0;
}

