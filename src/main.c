#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

int main() {
    char* cmdline;

    // Initialize readline
    rl_attempted_completion_function = shell_completion;
    rl_bind_key('\t', rl_complete);
    rl_completion_append_character = '\0';

    printf("========================================\n");
    printf("    FCIT Shell - I/O Redirection & Pipes\n");
    printf("========================================\n");
    printf("New Features:\n");
    printf("  • Input redirection:  command < file.txt\n");
    printf("  • Output redirection: command > file.txt\n");
    printf("  • Pipes:              cmd1 | cmd2 | cmd3\n");
    printf("========================================\n\n");

    using_history();
    read_history(".myshell_history");

    while ((cmdline = readline(PROMPT)) != NULL) {
        if (strlen(cmdline) == 0) {
            free(cmdline);
            continue;
        }

        // Handle history re-execution
        if (cmdline[0] == '!') {
            int num = atoi(cmdline + 1);
            HIST_ENTRY **hist = history_list();
            if (hist && num > 0 && num <= history_length) {
                free(cmdline);
                cmdline = strdup(hist[num - 1]->line);
                printf("%s\n", cmdline);
            } else {
                printf("No such command in history.\n");
                free(cmdline);
                continue;
            }
        }

        add_history(cmdline);

        // Parse command line into commands
        Command commands[MAX_PIPES];
        int num_commands = parse_command_line(cmdline, commands);
        
        if (num_commands > 0) {
            execute_parsed_commands(commands, num_commands);
        }
        
        // Cleanup
        free_commands(commands, num_commands);
        free(cmdline);
    }

    write_history(".myshell_history");
    clear_history();
    printf("\nShell exited.\n");
    return 0;
}
