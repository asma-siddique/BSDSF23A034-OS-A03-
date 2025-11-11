#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>  // ADD THIS for waitpid
#include <ctype.h>

#define MAX_ARGS 64
#define MAX_HISTORY 100
#define MAX_JOBS 64
#define INPUT_BUFFER_SIZE 1024

typedef struct History {
    char* commands[MAX_HISTORY];
    int count;
} History;

typedef struct Job {
    pid_t pid;
    char command[INPUT_BUFFER_SIZE];
    int running;
} Job;

typedef struct Var {
    char *name;
    char *value;
    struct Var *next;
} Var;

// History
void init_history(void);
void add_to_history(const char* cmd);
void show_history(void);
char* get_history_command(int n);

// Jobs
void init_jobs(void);
void add_job(pid_t pid, const char* cmd);
void remove_job(pid_t pid);
void cleanup_zombies(void);
void show_jobs(void);

// Built-ins & execution
int handle_builtin(char **args);
void execute_command(char *cmdline, int background);

// Utilities
void trim(char *str);
char* expand_variables(const char *cmdline);

// Variables
void set_variable(const char *name, const char *value);
char* get_variable(const char *name);
void show_variables(void);

#endif
