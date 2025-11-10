#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char* cmdline;

    init_history();
    init_jobs();
    
    printf("========================================\n");
    printf("  FCIT Shell - Command Chaining & Jobs\n");
    printf("========================================\n");
    printf("Type 'exit' to quit\n\n");

    while (1) {
        cleanup_zombies();
        
        printf(SHELL_PROMPT);
        fflush(stdout);
        
        cmdline = read_cmd();
        if (cmdline == NULL) continue;

        if (strcmp(cmdline, "exit") == 0) {
            free(cmdline);
            break;
        }

        add_to_history(cmdline);

        Command commands[MAX_COMMANDS];
        int num_commands = parse_command_line(cmdline, commands);
        
        if (num_commands > 0) {
            for (int i = 0; i < num_commands; i++) {
                execute_command(&commands[i]);
            }
        }
        
        free_commands(commands, num_commands);
        free(cmdline);
    }

    printf("Shell exited.\n");
    return 0;
}
