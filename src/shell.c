#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

static History history;
static Job jobs[MAX_JOBS];
static int job_count = 0;
static int next_job_id = 1;

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
char* read_cmd(void) {
    char buffer[INPUT_BUFFER_SIZE];
    
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

// Initialize jobs array
void init_jobs(void) {
    for (int i = 0; i < MAX_JOBS; i++) {
        jobs[i].pid = -1;
        jobs[i].command = NULL;
        jobs[i].job_id = 0;
    }
    job_count = 0;
    next_job_id = 1;
}

// Add a background job
void add_job(pid_t pid, const char* command) {
    if (job_count >= MAX_JOBS) {
        printf("Warning: Maximum jobs limit reached\n");
        return;
    }
    
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].pid == -1) {
            jobs[i].pid = pid;
            jobs[i].command = strdup(command);
            jobs[i].job_id = next_job_id++;
            job_count++;
            printf("[%d] %d\n", jobs[i].job_id, pid);
            return;
        }
    }
}

// Remove a completed job
void remove_job(pid_t pid) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].pid == pid) {
            free(jobs[i].command);
            jobs[i].pid = -1;
            jobs[i].command = NULL;
            jobs[i].job_id = 0;
            job_count--;
            return;
        }
    }
}

// Clean up zombie processes using waitpid(WNOHANG)
void cleanup_zombies(void) {
    int status;
    pid_t pid;
    
    // Non-blocking wait to reap any completed child processes
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        remove_job(pid);
    }
}

// Show all background jobs
void show_jobs(void) {
    printf("Jobs:\n");
    int found = 0;
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].pid != -1) {
            // Check if job is still running
            int result = waitpid(jobs[i].pid, NULL, WNOHANG);
            if (result == 0) {
                // Job is still running
                printf("[%d] Running %d %s\n", jobs[i].job_id, jobs[i].pid, jobs[i].command);
                found = 1;
            } else {
                // Job has completed
                remove_job(jobs[i].pid);
            }
        }
    }
    if (!found) {
        printf("No background jobs\n");
    }
}

// Enhanced parser to handle ; and &
int parse_command_line(char* line, Command* commands) {
    char** tokens = tokenize(line);
    int cmd_count = 0;
    int token_idx = 0;
    
    // Initialize first command
    commands[cmd_count].argc = 0;
    commands[cmd_count].input_file = NULL;
    commands[cmd_count].output_file = NULL;
    commands[cmd_count].background = 0;
    
    while (tokens[token_idx] != NULL) {
        if (strcmp(tokens[token_idx], "|") == 0) {
            cmd_count++;
            if (cmd_count >= MAX_COMMANDS) {
                fprintf(stderr, "Error: Too many commands (max %d)\n", MAX_COMMANDS);
                for (int i = 0; tokens[i] != NULL; i++) free(tokens[i]);
                free(tokens);
                return -1;
            }
            commands[cmd_count].argc = 0;
            commands[cmd_count].input_file = NULL;
            commands[cmd_count].output_file = NULL;
            commands[cmd_count].background = 0;
            token_idx++;
            continue;
        }
        else if (strcmp(tokens[token_idx], ";") == 0) {
            cmd_count++;
            if (cmd_count >= MAX_COMMANDS) {
                fprintf(stderr, "Error: Too many commands (max %d)\n", MAX_COMMANDS);
                for (int i = 0; tokens[i] != NULL; i++) free(tokens[i]);
                free(tokens);
                return -1;
            }
            commands[cmd_count].argc = 0;
            commands[cmd_count].input_file = NULL;
            commands[cmd_count].output_file = NULL;
            commands[cmd_count].background = 0;
            token_idx++;
            continue;
        }
        else if (strcmp(tokens[token_idx], "&") == 0) {
            commands[cmd_count].background = 1;
            token_idx++;
            continue;
        }
        else if (strcmp(tokens[token_idx], "<") == 0) {
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

// Execute command with background support
void execute_command(Command* cmd) {
    if (cmd->argc == 0) return;
    
    // Check if it's a built-in command first
    if (handle_builtin(cmd->args)) {
        return;
    }
    
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        if (cmd->input_file != NULL) {
            int fd_in = open(cmd->input_file, O_RDONLY);
            if (fd_in < 0) {
                perror("open input file");
                exit(1);
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }
        
        if (cmd->output_file != NULL) {
            int fd_out = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out < 0) {
                perror("open output file");
                exit(1);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }
        
        execvp(cmd->args[0], cmd->args);
        fprintf(stderr, "Command not found: %s\n", cmd->args[0]);
        exit(127);
    } 
    else if (pid > 0) {
        if (cmd->background) {
            // Background job
            add_job(pid, cmd->args[0]);
        } else {
            // Foreground job
            int status;
            waitpid(pid, &status, 0);
        }
    } 
    else {
        perror("fork failed");
    }
}

// Execute commands sequentially
void execute_parsed_commands(Command* commands, int num_commands) {
    for (int i = 0; i < num_commands; i++) {
        execute_command(&commands[i]);
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
        printf("Shell exited.\n");
        exit(0);
    }
    
    if (strcmp(arglist[0], "help") == 0) {
        printf("=== FCIT Shell - Multitasking ===\n");
        printf("Built-in commands: cd, exit, help, history, jobs\n");
        printf("Command Chaining: cmd1 ; cmd2 ; cmd3\n");
        printf("Background Execution: long_cmd &\n");
        printf("Job Control: jobs\n");
        return 1;
    }
    
    if (strcmp(arglist[0], "history") == 0) {
        show_history();
        return 1;
    }
    
    if (strcmp(arglist[0], "jobs") == 0) {
        show_jobs();
        return 1;
    }
    
    return 0;
}
