#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

// Use different names to avoid conflicts with system headers
#define SHELL_PROMPT "FCIT> "
#define INPUT_BUFFER_SIZE 1024  // Changed from MAX_INPUT
#define MAX_ARGS 64
#define MAX_PIPES 10
#define HISTORY_SIZE 50

// History structure
typedef struct {
    char* commands[HISTORY_SIZE];
    int count;
} History;

// Command structure for redirection and pipes
typedef struct {
    char* args[MAX_ARGS];
    char* input_file;
    char* output_file;
    int argc;
} Command;

// Function declarations
char** tokenize(char* line);
void execute_redirection(Command* cmd);
int handle_builtin(char** arglist);
char* read_cmd(const char* prompt);
void init_history(void);
void add_to_history(const char* cmd);
void show_history(void);
char* get_history_command(int n);
int parse_command_line(char* line, Command* commands);
void free_commands(Command* commands, int count);
void execute_parsed_commands(Command* commands, int num_commands);

#endif
