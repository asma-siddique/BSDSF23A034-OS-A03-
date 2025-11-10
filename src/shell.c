#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>

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

// Execute single command with I/O redirection
void execute_redirection(Command* cmd) {
    if (cmd->argc == 0) return; // Empty command
    
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
        // Parent process - wait for completion
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
    
    // Create pipes for the pipeline
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipefds + i * 2) < 0) {
            perror("pipe");
            return;
        }
    }
    
    // Fork child processes for each command
    for (int i = 0; i < num_commands; i++) {
        pids[i] = fork();
        
        if (pids[i] == 0) {
            // Child process
            
            // Connect to previous command's output (if not first command)
            if (i > 0) {
                if (dup2(pipefds[(i-1)*2], STDIN_FILENO) == -1) {
                    perror("dup2 pipe input");
                    exit(1);
                }
            }
            
            // Connect to next command's input (if not last command)
            if (i < num_commands - 1) {
                if (dup2(pipefds[i*2 + 1], STDOUT_FILENO) == -1) {
                    perror("dup2 pipe output");
                    exit(1);
                }
            }
            
            // Close all pipe file descriptors in child
            for (int j = 0; j < 2 * (num_commands - 1); j++) {
                close(pipefds[j]);
            }
            
            // Handle file redirection for this command (overrides pipes)
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
            
            // Execute the command
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
    
    // Wait for all child processes to complete
    for (int i = 0; i < num_commands; i++) {
        waitpid(pids[i], NULL, 0);
    }
}

// Execute parsed commands
void execute_parsed_commands(Command* commands, int num_commands) {
    if (num_commands == 1) {
        // Single command - check if it's a builtin first
        if (commands[0].argc > 0 && handle_builtin(commands[0].args)) {
            return;
        }
        // Not a builtin, execute with possible redirection
        execute_redirection(&commands[0]);
    } else {
        // Multiple commands - pipeline
        execute_pipeline(commands, num_commands);
    }
}

// Check for built-in commands
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
        printf("Examples:\n");
        printf("  ls -l > files.txt\n");
        printf("  sort < data.txt\n");
        printf("  cat file.txt | grep \"pattern\" | wc -l\n");
        return 1;
    }
    
    if (strcmp(arglist[0], "history") == 0) {
        HIST_ENTRY **hist = history_list();
        if (hist) {
            for (int i = 0; hist[i]; i++) {
                printf("%d %s\n", i + 1, hist[i]->line);
            }
        } else {
            printf("No command history.\n");
        }
        return 1;
    }
    
    if (strcmp(arglist[0], "jobs") == 0) {
        printf("Job control not yet implemented.\n");
        return 1;
    }
    
    return 0;
}

// Command generator for tab completion
char* command_generator(const char* text, int state) {
    static int list_index, len;
    const char *name;
    
    // List of built-in commands
    static const char* builtins[] = {
        "cd", "exit", "help", "history", "jobs", NULL
    };
    
    // Common system commands
    static const char* common_commands[] = {
        "ls", "pwd", "cat", "echo", "grep", "mkdir", "rm", "cp", "mv", "whoami", 
        "ps", "kill", "chmod", "chown", "find", "sort", "wc", "head", "tail", NULL
    };

    if (!state) {
        list_index = 0;
        len = strlen(text);
    }

    // First, check built-in commands
    while (builtins[list_index]) {
        name = builtins[list_index++];
        if (strncmp(name, text, len) == 0) {
            return strdup(name);
        }
    }

    // Then check common system commands
    list_index = 0;
    while (common_commands[list_index]) {
        name = common_commands[list_index++];
        if (strncmp(name, text, len) == 0) {
            return strdup(name);
        }
    }

    // Finally, check files in current directory
    static DIR *dir = NULL;
    static struct dirent *entry = NULL;
    
    if (!state) {
        if (dir) closedir(dir);
        dir = opendir(".");
    }
    
    if (dir) {
        while ((entry = readdir(dir)) != NULL) {
            name = entry->d_name;
            if (name[0] != '.' && strncmp(name, text, len) == 0) {
                return strdup(name);
            }
        }
        closedir(dir);
        dir = NULL;
    }

    return NULL;
}

// Completion function for Readline
char** shell_completion(const char* text, int start, int end) {
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, command_generator);
}
