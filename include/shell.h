#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>

// Tokenize input line (MODIFIED for Task 2)
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

// Parse command line into commands with redirection and pipes (NEW for Task 2)
int parse_command_line(char* line, Command* commands) {
    char** tokens = tokenize(line);
    int cmd_count = 0;
    int token_idx = 0;
    
    // Initialize first command
    commands[cmd_count].argc = 0;
    commands[cmd_count].input_file = NULL;
    commands[cmd_count].output_file = NULL;
    
    while (tokens[token_idx] != NULL) {
        if (strcmp(tokens[token_idx], "|") == 0) {
            // Pipe detected - move to next command
            cmd_count++;
            if (cmd_count >= MAX_PIPES) {
                fprintf(stderr, "Error: Too many pipes (max %d)\n", MAX_PIPES);
                free(tokens);
                return -1;
            }
            // Initialize new command
            commands[cmd_count].argc = 0;
            commands[cmd_count].input_file = NULL;
            commands[cmd_count].output_file = NULL;
            token_idx++;
            continue;
        }
        else if (strcmp(tokens[token_idx], "<") == 0) {
            // Input redirection
            if (tokens[token_idx + 1] == NULL) {
                fprintf(stderr, "Error: No input file specified after <\n");
                free(tokens);
                return -1;
            }
            commands[cmd_count].input_file = strdup(tokens[token_idx + 1]);
            token_idx += 2;
            continue;
        }
        else if (strcmp(tokens[token_idx], ">") == 0) {
            // Output redirection
            if (tokens[token_idx + 1] == NULL) {
                fprintf(stderr, "Error: No output file specified after >\n");
                free(tokens);
                return -1;
            }
            commands[cmd_count].output_file = strdup(tokens[token_idx + 1]);
            token_idx += 2;
            continue;
        }
        else {
            // Regular argument
            if (commands[cmd_count].argc < MAX_ARGS - 1) {
                commands[cmd_count].args[commands[cmd_count].argc] = strdup(tokens[token_idx]);
                commands[cmd_count].argc++;
            }
            token_idx++;
        }
    }
    
    // NULL terminate the argument list
    for (int i = 0; i <= cmd_count; i++) {
        commands[i].args[commands[i].argc] = NULL;
    }
    
    // Free tokens
    for (int i = 0; tokens[i] != NULL; i++) {
        free(tokens[i]);
    }
    free(tokens);
    
    return cmd_count + 1; // Return number of commands
}

// Free allocated memory in commands
void free_commands(Command* commands, int count) {
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < commands[i].argc; j++) {
            free(commands[i].args[j]);
        }
        if (commands[i].input_file) free(commands[i].input_file);
        if (commands[i].output_file) free(commands[i].output_file);
    }
}
