#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char* cmdline;
    char** arglist;

    // Initialize history
    init_history();
    
    printf("========================================\n");
    printf("     FCIT Shell - Feature 4 Ready\n");
    printf("========================================\n");
    printf("✓ Tab completion simulation\n");
    printf("✓ Command history\n"); 
    printf("✓ History re-execution (!n)\n");
    printf("✓ Built-in commands\n");
    printf("\nHow to use tab completion:\n");
    printf("  Type 'tab' then enter partial command\n");
    printf("  Example: Type 'tab', then 'ls'\n");
    printf("========================================\n\n");

    while ((cmdline = read_cmd(PROMPT)) != NULL) {
        if (strlen(cmdline) == 0) {
            free(cmdline);
            continue;
        }

        // Handle !n history re-execution
        char* actual_cmd = cmdline;
        if (cmdline[0] == '!') {
            int num = atoi(cmdline + 1);
            char* hist_cmd = get_history_command(num);
            if (hist_cmd) {
                free(cmdline);
                cmdline = strdup(hist_cmd);
                printf("[Re-executing] %s\n", cmdline);
                actual_cmd = cmdline;
            } else {
                printf("Error: No command %d in history. Use 'history' to see commands.\n", num);
                free(cmdline);
                continue;
            }
        }

        // Add to history
        if (strlen(actual_cmd) > 0) {
            add_to_history(actual_cmd);
        }

        // Execute command
        arglist = tokenize(actual_cmd);
        
        if (arglist[0] != NULL) {
            if (!handle_builtin(arglist)) {
                execute(arglist);
            }
        }

        // Cleanup
        for (int i = 0; arglist[i] != NULL; i++) {
            free(arglist[i]);
        }
        free(arglist);
        
        if (cmdline != actual_cmd) free(actual_cmd);
        free(cmdline);
    }

    printf("\nShell terminated.\n");
    return 0;
}
