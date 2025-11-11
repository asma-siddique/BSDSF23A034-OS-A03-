#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

static Job jobs[MAX_JOBS];
static int job_count = 0;

static char *history[MAX_HISTORY];
static int history_count = 0;

// ---------- Utility ----------
void trim(char *str) {
    int i = 0, j = strlen(str) - 1;
    while (isspace((unsigned char)str[i])) i++;
    while (j >= i && isspace((unsigned char)str[j])) j--;
    memmove(str, str + i, j - i + 1);
    str[j - i + 1] = '\0';
}

char *read_cmd(void) {
    char *cmdline = malloc(CMD_LEN);
    if (!fgets(cmdline, CMD_LEN, stdin)) {
        free(cmdline);
        return NULL;
    }
    cmdline[strcspn(cmdline, "\n")] = 0;
    return cmdline;
}

// ---------- Job Control ----------
void init_jobs() {
    job_count = 0;
}

void add_job(pid_t pid, const char *cmd) {
    if (job_count < MAX_JOBS) {
        jobs[job_count].pid = pid;
        jobs[job_count].running = 1;
        strncpy(jobs[job_count].command, cmd, CMD_LEN - 1);
        printf("[%d] %d running in background: %s\n", job_count + 1, pid, cmd);
        job_count++;
    }
}

void show_jobs() {
    printf("Jobs:\n");
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].running)
            printf("[%d] Running %d %s\n", i + 1, jobs[i].pid, jobs[i].command);
    }
}

void cleanup_zombies() {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < job_count; i++) {
            if (jobs[i].pid == pid) {
                jobs[i].running = 0;
                printf("\n[%d] Done %d %s\n", i + 1, pid, jobs[i].command);
                break;
            }
        }
    }
}

// ---------- Command Execution ----------
void execute_command(char *cmdline, int background) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return;
    }

    if (pid == 0) {
        char *args[CMD_LEN / 2 + 1];
        int i = 0;
        char *token = strtok(cmdline, " ");
        while (token != NULL && i < CMD_LEN / 2) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;

        execvp(args[0], args);
        perror("Command not found");
        exit(EXIT_FAILURE);
    } else {
        if (background) {
            add_job(pid, cmdline);
        } else {
            waitpid(pid, NULL, 0);
        }
    }
}

// ---------- History ----------
void init_history() {
    history_count = 0;
}

void add_to_history(const char *cmd) {
    if (history_count < MAX_HISTORY) {
        history[history_count++] = strdup(cmd);
    } else {
        free(history[0]);
        memmove(&history[0], &history[1], sizeof(char*) * (MAX_HISTORY - 1));
        history[MAX_HISTORY - 1] = strdup(cmd);
    }
}

void show_history() {
    for (int i = 0; i < history_count; i++) {
        printf("%d %s\n", i + 1, history[i]);
    }
}

char *get_history_command(int index) {
    if (index < 1 || index > history_count) return NULL;
    return history[index - 1];
}

