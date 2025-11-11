#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char* cmdline;

    init_history();
    
    printf("========================================\n");
    printf("    FCIT Shell - I/O Redirection & Pipes\n");
    printf("========================================\n");
    printf("Type 'exit' to quit\n\n");

    while (1) {
        // Show prompt and read input
        printf(SHELL_PROMPT);
        fflush(stdout);  // Force the prompt to display
        
        cmdline = read_cmd(SHELL_PROMPT);


        
        if (cmdline == NULL) {
            // This means EOF (Ctrl+D) or empty input
            printf("\n");
            break;
        }

        // Handle exit command
        if (strcmp(cmdline, "exit") == 0) {
            free(cmdline);
            break;
        }

        // Add to history
        add_to_history(cmdline);

        // Parse and execute
        Command commands[MAX_PIPES];
        int num_commands = parse_command_line(cmdline, commands);
        
        if (num_commands > 0) {
            execute_parsed_commands(commands, num_commands);
        } else {
            printf("Error: Failed to parse command\n");
        }
        
        // Cleanup
        free_commands(commands, num_commands);
        free(cmdline);
    }

    printf("Shell exited.\n");
    return 0;
}
