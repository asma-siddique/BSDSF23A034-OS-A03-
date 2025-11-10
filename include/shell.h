#ifndef SHELL_H
#define SHELL_H

// Standard headers in correct order
#include <stddef.h>      // MUST be first for size_t
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>

#define SHELL_PROMPT "FCIT> "
#define INPUT_BUFFER_SIZE 1024
#define MAX_ARGS 64
#define MAX_COMMANDS 20
#define MAX_JOBS 20
#define HISTORY_SIZE 50

// Job structure for background processes
typedef struct {
    pid_t pid;
    char* command;
    int job_id;
} Job;

// History structure
typedef struct {
    char* commands[HISTORY_SIZE];
    int count;
} History;

// Command structure
typedef struct {
    char* args[MAX_ARGS];
    char* input_file;
    char* output_file;
    int argc;
    int background;
} Command;

// Function declarations
char** tokenize(char* line);
void execute_command(Command* cmd);
int handle_builtin(char** arglist);
char* read_cmd(void);
void init_history(void);
void add_to_history(const char* cmd);
void show_history(void);
char* get_history_command(int n);
int parse_command_line(char* line, Command* commands);
void free_commands(Command* commands, int count);

// Feature-6 functions
void init_jobs(void);
void add_job(pid_t pid, const char* command);
void remove_job(pid_t pid);
void cleanup_zombies(void);
void show_jobs(void);

#endif
