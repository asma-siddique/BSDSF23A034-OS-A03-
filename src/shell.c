#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simple input function
char* read_cmd(void) {
    char buffer[INPUT_BUFFER_SIZE];
    
    printf(SHELL_PROMPT);
    fflush(stdout);
    
    if (fgets(buffer, INPUT_BUFFER_SIZE, stdin) == NULL) {
        return NULL;
    }
    
    buffer[strcspn(buffer, "\n")] = 0;
    return (strlen(buffer) > 0) ? strdup(buffer) : NULL;
}

// Tokenize input line
char** tokenize(char* line) {
    int size = 64, idx = 0;
    char** tokens = malloc(size * sizeof(char*));
    char* token = strtok(line, " \t\n");
    
    while (token) {
        tokens[idx++] = strdup(token);
        if (idx >= size) {
            size *= 2;
            tokens = realloc(tokens, size * sizeof(char*));
        }
        token = strtok(NULL, " \t\n");
    }
    tokens[idx] = NULL;
    return tokens;
}

// Simple command execution
void execute_command(char** args) {
    if (!args[0]) return;
    
    if (handle_builtin(args)) {
        return;
    }
    
    pid_t pid = fork();
    
    if (pid == 0) {
        execvp(args[0], args);
        fprintf(stderr, "Command not found: %s\n", args[0]);
        exit(127);
    } 
    else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } 
    else {
        perror("fork failed");
    }
}

// Handle built-in commands
int handle_builtin(char** arglist) {
    if (!arglist[0]) return 0;
    
    if (strcmp(arglist[0], "cd") == 0) {
        if (arglist[1] == NULL) {
            char* home = getenv("HOME");
            if (home) chdir(home);
        } else {
            chdir(arglist[1]);
        }
        return 1;
    }
    
    if (strcmp(arglist[0], "exit") == 0) {
        printf("Shell exited.\n");
        exit(0);
    }
    
    if (strcmp(arglist[0], "help") == 0) {
        printf("=== FCIT Shell - If-Then-Else Support ===\n");
        printf("Built-in commands: cd, exit, help\n");
        printf("Control Structures:\n");
        printf("  if condition\n");
        printf("  then\n");
        printf("    commands...\n");
        printf("  else\n");
        printf("    commands...\n");
        printf("  fi\n");
        return 1;
    }
    
    return 0;
}

// Initialize if block
void init_if_block(IfBlock* if_block) {
    if_block->condition = NULL;
    if_block->then_count = 0;
    if_block->else_count = 0;
    if_block->in_if_block = 0;
    if_block->in_then_block = 0;
    if_block->in_else_block = 0;
    
    for (int i = 0; i < MAX_BLOCK_LINES; i++) {
        if_block->then_commands[i] = NULL;
        if_block->else_commands[i] = NULL;
    }
}

// Parse if-then-else-fi structure
int parse_if_statement(char* line, IfBlock* if_block) {
    line[strcspn(line, "\n")] = 0;
    
    if (strlen(line) == 0) {
        return 1;
    }
    
    if (strncmp(line, "if ", 3) == 0 && !if_block->in_if_block) {
        if_block->in_if_block = 1;
        if_block->condition = strdup(line + 3);
        return 1;
    }
    
    if (strcmp(line, "then") == 0 && if_block->in_if_block && !if_block->in_then_block) {
        if_block->in_then_block = 1;
        return 1;
    }
    
    if (strcmp(line, "else") == 0 && if_block->in_then_block) {
        if_block->in_then_block = 0;
        if_block->in_else_block = 1;
        return 1;
    }
    
    if (strcmp(line, "fi") == 0 && if_block->in_if_block) {
        return 0;
    }
    
    if (if_block->in_then_block && if_block->then_count < MAX_BLOCK_LINES - 1) {
        if_block->then_commands[if_block->then_count++] = strdup(line);
        return 1;
    }
    
    if (if_block->in_else_block && if_block->else_count < MAX_BLOCK_LINES - 1) {
        if_block->else_commands[if_block->else_count++] = strdup(line);
        return 1;
    }
    
    if (if_block->in_if_block) {
        fprintf(stderr, "Error: Unexpected line in if statement: %s\n", line);
        return -1;
    }
    
    return 0;
}

// Execute if-then-else block
void execute_if_block(IfBlock* if_block) {
    if (!if_block->condition) {
        fprintf(stderr, "Error: No condition in if statement\n");
        return;
    }
    
    pid_t pid = fork();
    int status = 0;
    
    if (pid == 0) {
        // Child process - execute condition
        char** args = tokenize(if_block->condition);
        execvp(args[0], args);
        fprintf(stderr, "Command not found: %s\n", args[0]);
        
        // Free tokens
        for (int i = 0; args[i] != NULL; i++) {
            free(args[i]);
        }
        free(args);
        
        exit(127);
    } 
    else if (pid > 0) {
        // Parent process - wait for condition result
        waitpid(pid, &status, 0);
        int exit_status = WEXITSTATUS(status);
        
        // Execute appropriate block based on exit status
        if (exit_status == 0) {
            // Success - execute then block
            for (int i = 0; i < if_block->then_count && if_block->then_commands[i]; i++) {
                printf("FCIT> %s\n", if_block->then_commands[i]);
                char** args = tokenize(if_block->then_commands[i]);
                execute_command(args);
                
                // Free tokens
                for (int j = 0; args[j] != NULL; j++) {
                    free(args[j]);
                }
                free(args);
            }
        } 
        else {
            // Failure - execute else block (if exists)
            for (int i = 0; i < if_block->else_count && if_block->else_commands[i]; i++) {
                printf("FCIT> %s\n", if_block->else_commands[i]);
                char** args = tokenize(if_block->else_commands[i]);
                execute_command(args);
                
                // Free tokens
                for (int j = 0; args[j] != NULL; j++) {
                    free(args[j]);
                }
                free(args);
            }
        }
    } 
    else {
        perror("fork failed");
    }
}

// Enhanced read_cmd for if-then-else
char* read_cmd_if(void) {
    static char buffer[INPUT_BUFFER_SIZE];
    static IfBlock if_block;
    static int in_if_mode = 0;
    
    if (!in_if_mode) {
        // Normal mode - read single line
        printf(SHELL_PROMPT);
        fflush(stdout);
        
        if (fgets(buffer, INPUT_BUFFER_SIZE, stdin) == NULL) {
            return NULL;
        }
        
        buffer[strcspn(buffer, "\n")] = 0;
        
        // Check if this starts an if statement
        if (strncmp(buffer, "if ", 3) == 0) {
            in_if_mode = 1;
            init_if_block(&if_block);
            parse_if_statement(buffer, &if_block);
            return NULL; // Continue reading multi-line
        }
        
        return (strlen(buffer) > 0) ? strdup(buffer) : NULL;
    } 
    else {
        // Multi-line if statement mode
        printf("> ");
        fflush(stdout);
        
        if (fgets(buffer, INPUT_BUFFER_SIZE, stdin) == NULL) {
            in_if_mode = 0;
            return NULL;
        }
        
        int result = parse_if_statement(buffer, &if_block);
        
        if (result == 0) {
            // if block complete
            in_if_mode = 0;
            execute_if_block(&if_block);
            
            // Cleanup
            free(if_block.condition);
            for (int i = 0; i < if_block.then_count; i++) {
                free(if_block.then_commands[i]);
            }
            for (int i = 0; i < if_block.else_count; i++) {
                free(if_block.else_commands[i]);
            }
            init_if_block(&if_block);
            
            return NULL;
        } 
        else if (result == -1) {
            // Error in if statement
            in_if_mode = 0;
            init_if_block(&if_block);
            return NULL;
        }
        
        // Continue reading multi-line
        return NULL;
    }
}
