#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char* cmdline;

    // Initialize history and background jobs
    init_history();
    init_jobs();

    printf("========================================\n");
    printf("  FCIT Shell - Command Chaining & Jobs\n");
    printf("========================================\n");
    printf("Type 'exit' to quit\n\n");

    while (1) {
        // Reap any finished background processes
        cleanup_zombies();

        // Show prompt and read input
        printf(SHELL_PROMPT);
        fflush(stdout);

        cmdline = read_cmd();
        if (cmdline == NULL) {
            // EOF (Ctrl+D) or empty input
            printf("\n");
            break;
        }

        // Exit command
        if (strcmp(cmdline, "exit") == 0) {
            free(cmdline);
            break;
        }

        // Add command to history
        add_to_history(cmdline);

        // Parse commands (handles pipelines, redirection, etc.)
        Command commands[MAX_COMMANDS];
        int num_commands = parse_command_line(cmdline, commands);

        if (num_commands > 0) {
            for (int i = 0; i < num_commands; i++) {
                execute_command(&commands[i]);
            }
        }

        // Cleanup
        free_commands(commands, num_commands);
        free(cmdline);
    }

    printf("Shell exited.\n");
    return 0;
}

