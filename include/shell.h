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

#define SHELL_PROMPT "FCIT> "
#define INPUT_BUFFER_SIZE 1024
#define MAX_ARGS 64
#define MAX_PIPES 10
#define HISTORY_SIZE 50
#define MAX_JOBS 50

typedef struct {
    char* commands[HISTORY_SIZE];
    int count;
} History;

typedef struct {
    char* args[MAX_ARGS];
    char* input_file;
    char* output_file;
    int argc;
    int background; // 1 if command ends with &
} Command;

typedef struct {
    pid_t pid;
    char cmd[INPUT_BUFFER_SIZE];
    int running; // 1 running, 0 finished
} Job;

// History functions
void init_history(void);
void add_to_history(const char* cmd);
void show_history(void);
char* get_history_command(int n);

// Built-in and execution
int handle_builtin(char** arglist);
int execute(Command* cmd);
void execute_parsed_commands(Command* commands, int num_commands);
int parse_command_line(char* line, Command* commands);
void free_commands(Command* commands, int count);

// Command reading
char* read_cmd(const char* prompt);

// Job control
void init_jobs(void);
void add_job(pid_t pid, const char* cmd);
void remove_job(pid_t pid);
void cleanup_zombies(void);
void show_jobs(void);

#endif

