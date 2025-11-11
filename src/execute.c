#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

// Check if a string is a valid variable assignment
int is_assignment(const char *str) {
    if (!str) return 0;
    
    // Must contain '=' and not start with '='
    char *eq = strchr(str, '=');
    if (!eq || eq == str) return 0;
    
    // Check variable name (before '=')
    for (const char *p = str; p < eq; p++) {
        if (!isalnum(*p) && *p != '_') {
            return 0;
        }
    }
    
    return 1;
}

// Handle variable assignment
int handle_assignment(char *arg) {
    if (!is_assignment(arg)) return 0;
    
    char *eq = strchr(arg, '=');
    *eq = '\0';  // Split into name and value
    
    char *name = arg;
    char *value = eq + 1;
    
    // Remove quotes if present
    if (value[0] == '"' && value[strlen(value)-1] == '"') {
        value[strlen(value)-1] = '\0';
        value++;
    } else if (value[0] == '\'' && value[strlen(value)-1] == '\'') {
        value[strlen(value)-1] = '\0';
        value++;
    }
    
    set_variable(name, value);
    return 1;
}

// Built-in handler
int handle_builtin(char** args) {
    if (!args || !args[0]) return 0;

    if (strcmp(args[0], "cd") == 0) {
        if (!args[1]) {
            char *home = getenv("HOME");
            if (home) chdir(home);
        } else if (chdir(args[1]) != 0) {
            perror("cd failed");
        }
        return 1;
    }

    if (strcmp(args[0], "exit") == 0) {
        printf("Shell exited.\n");
        exit(0);
    }

    if (strcmp(args[0], "help") == 0) {
        printf("=== FCIT Shell ===\n");
        printf("Built-in commands: cd, exit, help, history, jobs, set\n");
        printf("Variable assignment: VARNAME=value\n");
        printf("Variable expansion: echo $VARNAME\n");
        return 1;
    }

    if (strcmp(args[0], "history") == 0) {
        show_history();
        return 1;
    }

    if (strcmp(args[0], "jobs") == 0) {
        show_jobs();
        return 1;
    }

    if (strcmp(args[0], "set") == 0) {
        show_variables();
        return 1;
    }

    // Check for variable assignment
    if (is_assignment(args[0])) {
        handle_assignment(args[0]);
        return 1;
    }

    return 0;
}

// Tokenize command line
char** tokenize_command(char *cmdline) {
    if (!cmdline) return NULL;
    
    char **args = malloc(MAX_ARGS * sizeof(char*));
    if (!args) return NULL;
    
    int argc = 0;
    char *token = strtok(cmdline, " \t");
    
    while (token && argc < MAX_ARGS - 1) {
        args[argc++] = strdup(token);
        token = strtok(NULL, " \t");
    }
    args[argc] = NULL;
    
    return args;
}

// Free tokens
void free_tokens(char **args) {
    if (!args) return;
    
    for (int i = 0; args[i]; i++) {
        free(args[i]);
    }
    free(args);
}

// Execute a command line
void execute_command(char *cmdline, int background) {
    if (!cmdline || strlen(cmdline) == 0) return;

    // Expand variables first
    char *expanded = expand_variables(cmdline);
    if (!expanded) return;

    // Tokenize the expanded command
    char **args = tokenize_command(expanded);
    if (!args) {
        free(expanded);
        return;
    }

    if (!args[0]) {
        free_tokens(args);
        free(expanded);
        return;
    }

    // Handle built-ins and assignments
    if (handle_builtin(args)) {
        free_tokens(args);
        free(expanded);
        return;
    }

    // Execute external command
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
    } else if (pid == 0) {
        // Child process
        execvp(args[0], args);
        fprintf(stderr, "Command not found: %s\n", args[0]);
        exit(127);
    } else {
        // Parent process
        if (background) {
            add_job(pid, cmdline);
        } else {
            waitpid(pid, NULL, 0);
        }
    }

    free_tokens(args);
    free(expanded);
}
