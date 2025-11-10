#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

static History history;

// Initialize history
void init_history(void) {
    history.count = 0;
}

// Add command to history
void add_to_history(const char* cmd) {
    if (history.count >= HISTORY_SIZE) {
        free(history.commands[0]);
        for (int i = 1; i < history.count; i++) {
            history.commands[i-1] = history.commands[i];
        }
        history.count--;
    }
    history.commands[history.count++] = strdup(cmd);
}

// Show history
void show_history(void) {
    printf("Command History:\n");
    for (int i = 0; i < history.count; i++) {
        printf("%d: %s\n", i + 1, history.commands[i]);
    }
}

// Get command from history
char* get_history_command(int n) {
    if (n < 1 || n > history.count) return NULL;
    return history.commands[n-1];
}

// Simple input function
char* read_cmd(const char* prompt) {
    printf("%s", prompt);
    fflush(stdout);
    
    static char buffer[INPUT_BUFFER_SIZE];
    if (fgets(buffer, INPUT_BUFFER_SIZE, stdin) == NULL) {
        return NULL;
    }
    
    // Remove newline
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

// Parse command line into commands with redirection and pipes
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
                for (int i = 0; tokens[i] != NULL; i++) free(tokens[i]);
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
                for (int i = 0; tokens[i] != NULL; i++) free(tokens[i]);
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
                for (int i = 0; tokens[i] != NULL; i++) free(tokens[i]);
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
    
    // NULL terminate the argument list for each command
    for (int i = 0; i <= cmd_count; i++) {
        commands[i].args[commands[i].argc] = NULL;
    }
    
    // Free tokens
    for (int i = 0; tokens[i] != NULL; i++) {
        free(tokens[i]);
    }
    free(tokens);
    
    return cmd_count + 1;
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

// Execute single command with I/O redirection
void execute_redirection(Command* cmd) {
    if (cmd->argc == 0) return;
    
    // Check if it's a built-in command first
    if (handle_builtin(cmd->args)) {
        return;
    }
    
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process - handle redirection
        
        // Input redirection
        if (cmd->input_file != NULL) {
            int fd_in = open(cmd->input_file, O_RDONLY);
            if (fd_in < 0) {
                perror("open input file");
                exit(1);
            }
            if (dup2(fd_in, STDIN_FILENO) == -1) {
                perror("dup2 input");
                exit(1);
            }
            close(fd_in);
        }
        
        // Output redirection
        if (cmd->output_file != NULL) {
            int fd_out = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out < 0) {
                perror("open output file");
                exit(1);
            }
            if (dup2(fd_out, STDOUT_FILENO) == -1) {
                perror("dup2 output");
                exit(1);
            }
            close(fd_out);
        }
        
        // Execute the command
        execvp(cmd->args[0], cmd->args);
        fprintf(stderr, "Command not found: %s\n", cmd->args[0]);
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

// Execute pipeline of commands
void execute_pipeline(Command* commands, int num_commands) {
    int pipefds[2 * (num_commands - 1)];
    pid_t pids[num_commands];
    
    // Create pipes
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipefds + i * 2) < 0) {
            perror("pipe");
            return;
        }
    }
    
    // Fork child processes
    for (int i = 0; i < num_commands; i++) {
        pids[i] = fork();
        
        if (pids[i] == 0) {
            // Child process
            
            // Connect to previous command's output
            if (i > 0) {
                if (dup2(pipefds[(i-1)*2], STDIN_FILENO) == -1) {
                    perror("dup2 pipe input");
                    exit(1);
                }
            }
            
            // Connect to next command's input
            if (i < num_commands - 1) {
                if (dup2(pipefds[i*2 + 1], STDOUT_FILENO) == -1) {
                    perror("dup2 pipe output");
                    exit(1);
                }
            }
            
            // Close all pipe file descriptors
            for (int j = 0; j < 2 * (num_commands - 1); j++) {
                close(pipefds[j]);
            }
            
            // Handle file redirection
            if (commands[i].input_file != NULL) {
                int fd_in = open(commands[i].input_file, O_RDONLY);
                if (fd_in < 0) {
                    perror("open input file");
                    exit(1);
                }
                dup2(fd_in, STDIN_FILENO);
                close(fd_in);
            }
            
            if (commands[i].output_file != NULL) {
                int fd_out = open(commands[i].output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd_out < 0) {
                    perror("open output file");
                    exit(1);
                }
                dup2(fd_out, STDOUT_FILENO);
                close(fd_out);
            }
            
            // Execute command
            execvp(commands[i].args[0], commands[i].args);
            fprintf(stderr, "Command not found: %s\n", commands[i].args[0]);
            exit(127);
        }
        else if (pids[i] < 0) {
            perror("fork");
            return;
        }
    }
    
    // Parent process - close all pipe ends
    for (int i = 0; i < 2 * (num_commands - 1); i++) {
        close(pipefds[i]);
    }
    
    // Wait for all children
    for (int i = 0; i < num_commands; i++) {
        waitpid(pids[i], NULL, 0);
    }
}

// Execute parsed commands
void execute_parsed_commands(Command* commands, int num_commands) {
    if (num_commands == 1) {
        execute_redirection(&commands[0]);
    } else {
        execute_pipeline(commands, num_commands);
    }
}

// Handle built-in commands
int handle_builtin(char** arglist) {
    if (!arglist[0]) return 0;
    
    if (strcmp(arglist[0], "cd") == 0) {
        if (arglist[1] == NULL) {
            char* home = getenv("HOME");
            if (home) {
                if (chdir(home) != 0) {
                    perror("cd");
                }
            }
        } else {
            if (chdir(arglist[1]) != 0) {
                perror("cd");
            }
        }
        return 1;
    }
    
    if (strcmp(arglist[0], "exit") == 0) {
        // Cleanup history
        for (int i = 0; i < history.count; i++) {
            free(history.commands[i]);
        }
        printf("Shell exited.\n");
        exit(0);
    }
    
    if (strcmp(arglist[0], "help") == 0) {
        printf("=== FCIT Shell with I/O Redirection & Pipes ===\n");
        printf("Built-in commands: cd, exit, help, history, jobs\n");
        printf("I/O Redirection:\n");
        printf("  command < input.txt    - Read input from file\n");
        printf("  command > output.txt   - Write output to file\n");
        printf("Pipes:\n");
        printf("  cmd1 | cmd2 | cmd3     - Chain commands together\n");
        return 1;
    }
    
    if (strcmp(arglist[0], "history") == 0) {
        show_history();
        return 1;
    }
    
    if (strcmp(arglist[0], "jobs") == 0) {
        printf("Job control not yet implemented.\n");
        return 1;
    }
    
    return 0;
}
