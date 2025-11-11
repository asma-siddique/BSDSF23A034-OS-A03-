#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LEN     1024
#define MAXARGS     64
#define ARGLEN      64
#define PROMPT      "FCIT> "

/* ---------- History configuration ---------- */
#define HISTORY_SIZE 20

extern char* history[HISTORY_SIZE];
extern int history_count;

/* ---------- Core shell functions ---------- */
char* read_cmd(char* prompt, FILE* fp);
char** tokenize(char* cmdline);
int execute(char** arglist);
int handle_builtin(char** arglist);

/* ---------- History helper functions ---------- */
void add_to_history(const char* cmd);
void show_history(void);
char* get_history_command(int n);

#endif

