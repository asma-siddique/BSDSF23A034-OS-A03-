#ifndef SHELL_H
#define SHELL_H

#define SHELL_PROMPT "FCIT> "
#define MAX_JOBS 100
#define MAX_HISTORY 50
#define CMD_LEN 1024

#include <sys/types.h>

typedef struct {
    pid_t pid;
    char command[CMD_LEN];
    int running;
} Job;

void trim(char *str);
char *read_cmd(void);
void execute_command(char *cmdline, int background);

// Job control
void init_jobs();
void add_job(pid_t pid, const char *cmd);
void show_jobs();
void cleanup_zombies();

// History
void init_history();
void add_to_history(const char *cmd);
void show_history();
char *get_history_command(int index);

#endif

