#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char* cmdline;
    Command commands[MAX_PIPES];
    int num_commands;

    init_history();
    init_jobs();

    printf("========================================\n");
    printf("    FCIT Shell - Feature 6 (Jobs & History)\n");
    printf("========================================\n");
    printf("Type 'exit' to quit\n\n");

    while (1) {
        cleanup_zombies();
        cmdline = read_cmd(SHELL_PROMPT);
        if (!cmdline) continue;

        // Handle !n history recall
        if (cmdline[0] == '!' && strlen(cmdline) > 1) {
            int n = atoi(cmdline + 1);
            char* hist_cmd = get_history_command(n);
            if (!hist_cmd) {
                printf("No such command in history.\n");
                free(cmdline);
                continue;
            }
            free(cmdline);
            cmdline = strdup(hist_cmd);
            printf("%s\n", cmdline);
        }

        add_to_history(cmdline);

        num_commands = parse_command_line(cmdline, commands);
        if (num_commands > 0) execute_parsed_commands(commands, num_commands);

        free(cmdline);
    }

    return 0;
}

