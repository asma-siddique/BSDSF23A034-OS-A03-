#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static History history;
static Job jobs[MAX_JOBS];
static Var *variables = NULL;

// ---------------- History ----------------
void init_history(void) {
    history.count = 0;
    for (int i = 0; i < MAX_HISTORY; i++) history.commands[i] = NULL;
}

void add_to_history(const char* cmd) {
    if (history.count < MAX_HISTORY) {
        history.commands[history.count++] = strdup(cmd);
    } else {
        free(history.commands[0]);
        memmove(history.commands, history.commands + 1, sizeof(char*) * (MAX_HISTORY - 1));
        history.commands[MAX_HISTORY - 1] = strdup(cmd);
    }
}

void show_history(void) {
    printf("Command History:\n");
    for (int i = 0; i < history.count; i++)
        printf("%d: %s\n", i+1, history.commands[i]);
}

char* get_history_command(int n) {
    if (n <= 0 || n > history.count) return NULL;
    return history.commands[n-1];
}

// ---------------- Jobs ----------------
void init_jobs(void) {
    for (int i = 0; i < MAX_JOBS; i++) jobs[i].running = 0;
}

void add_job(pid_t pid, const char* cmd) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (!jobs[i].running) {
            jobs[i].pid = pid;
            strncpy(jobs[i].command, cmd, INPUT_BUFFER_SIZE);
            jobs[i].running = 1;
            printf("[%d] %d\n", i+1, pid);
            return;
        }
    }
}

void remove_job(pid_t pid) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].running && jobs[i].pid == pid) {
            jobs[i].running = 0;
            return;
        }
    }
}

void cleanup_zombies(void) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        remove_job(pid);
    }
}

void show_jobs(void) {
    int found = 0;
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].running) {
            int result = waitpid(jobs[i].pid, NULL, WNOHANG);
            if (result == 0) {
                printf("[%d] Running %d %s\n", i+1, jobs[i].pid, jobs[i].command);
                found = 1;
            } else {
                remove_job(jobs[i].pid);
            }
        }
    }
    if (!found) printf("No background jobs\n");
}

// ---------------- Variables ----------------
void set_variable(const char *name, const char *value) {
    // Validate variable name
    if (!name || strlen(name) == 0) return;
    
    // Check if name contains only alphanumeric and underscore
    for (int i = 0; name[i]; i++) {
        if (!isalnum(name[i]) && name[i] != '_') {
            fprintf(stderr, "Invalid variable name: %s\n", name);
            return;
        }
    }

    Var *v = variables;
    while (v) {
        if (strcmp(v->name, name) == 0) {
            free(v->value);
            v->value = strdup(value);
            return;
        }
        v = v->next;
    }
    
    // Create new variable
    Var *newv = malloc(sizeof(Var));
    newv->name = strdup(name);
    newv->value = strdup(value);
    newv->next = variables;
    variables = newv;
}

char* get_variable(const char *name) {
    Var *v = variables;
    while (v) {
        if (strcmp(v->name, name) == 0) return v->value;
        v = v->next;
    }
    return "";
}

void show_variables(void) {
    Var *v = variables;
    if (!v) {
        printf("No variables defined\n");
        return;
    }
    printf("Shell variables:\n");
    while (v) {
        printf("  %s=%s\n", v->name, v->value);
        v = v->next;
    }
}

// ---------------- Utilities ----------------
void trim(char *str) {
    if (!str) return;
    
    char *end;
    
    // Trim leading space
    while(isspace((unsigned char)*str)) str++;
    
    if(*str == 0) return;
    
    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    
    end[1] = '\0';
}

char* expand_variables(const char *cmdline) {
    if (!cmdline) return strdup("");
    
    char *result = malloc(INPUT_BUFFER_SIZE);
    if (!result) return NULL;
    
    result[0] = '\0';
    const char *p = cmdline;
    char buffer[INPUT_BUFFER_SIZE] = {0};
    int buf_idx = 0;
    
    while (*p) {
        if (*p == '$') {
            // If we have accumulated regular text, add it to result
            if (buf_idx > 0) {
                buffer[buf_idx] = '\0';
                strcat(result, buffer);
                buf_idx = 0;
            }
            
            p++; // Skip '$'
            
            // Extract variable name
            char varname[64] = {0};
            int var_idx = 0;
            
            if (*p == '{') {
                // Handle ${VAR} syntax
                p++;
                while (*p && *p != '}') {
                    varname[var_idx++] = *p++;
                }
                if (*p == '}') p++;
            } else {
                // Handle $VAR syntax
                while (*p && (isalnum(*p) || *p == '_')) {
                    varname[var_idx++] = *p++;
                }
            }
            
            varname[var_idx] = '\0';
            
            // Get variable value
            char *val = get_variable(varname);
            if (val && strlen(val) > 0) {
                strcat(result, val);
            }
        } else {
            // Accumulate regular characters
            if (buf_idx < INPUT_BUFFER_SIZE - 1) {
                buffer[buf_idx++] = *p;
            }
            p++;
        }
    }
    
    // Add any remaining accumulated text
    if (buf_idx > 0) {
        buffer[buf_idx] = '\0';
        strcat(result, buffer);
    }
    
    return result;
}
