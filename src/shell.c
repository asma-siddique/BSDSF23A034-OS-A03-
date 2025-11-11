#include "shell.h"

// ------------------ History -------------------
static History history;

void init_history(void) {
    history.count = 0;
    for (int i = 0; i < HISTORY_SIZE; i++) history.commands[i] = NULL;
}

void add_to_history(const char* cmd) {
    if (history.count < HISTORY_SIZE) {
        history.commands[history.count++] = strdup(cmd);
    } else {
        free(history.commands[0]);
        memmove(history.commands, history.commands + 1, sizeof(char*) * (HISTORY_SIZE - 1));
        history.commands[HISTORY_SIZE-1] = strdup(cmd);
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

// ------------------ Jobs -------------------
static Job jobs[MAX_JOBS];
static int job_count = 0;

void init_jobs(void) {
    job_count = 0;
    for (int i = 0; i < MAX_JOBS; i++) jobs[i].running = 0;
}

void add_job(pid_t pid, const char* cmd) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (!jobs[i].running) {
            jobs[i].pid = pid;
            strncpy(jobs[i].cmd, cmd, INPUT_BUFFER_SIZE);
            jobs[i].running = 1;
            job_count++;
            printf("[%d] %d\n", i+1, pid);
            return;
        }
    }
}

void remove_job(pid_t pid) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].running && jobs[i].pid == pid) {
            jobs[i].running = 0;
            job_count--;
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
    printf("Jobs:\n");
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].running) printf("[%d] Running %d %s\n", i+1, jobs[i].pid, jobs[i].cmd);
    }
}

// ------------------ Read command -------------------
char* read_cmd(const char* prompt) {
    static char buffer[INPUT_BUFFER_SIZE];
    if (prompt) printf("%s", prompt);
    if (!fgets(buffer, sizeof(buffer), stdin)) return NULL;

    size_t len = strlen(buffer);
    if (len > 0 && buffer[len-1] == '\n') buffer[len-1] = '\0';
    return strdup(buffer); // caller must free
}

// ------------------ Parse & execute -------------------
int parse_command_line(char* line, Command* commands) {
    int num_commands = 0;
    char* saveptr1 = NULL;
    char* token1 = strtok_r(line, ";", &saveptr1);
    while (token1 && num_commands < MAX_PIPES) {
        Command* cmd = &commands[num_commands];
        cmd->argc = 0;
        cmd->input_file = NULL;
        cmd->output_file = NULL;
        cmd->background = 0;

        char* saveptr2 = NULL;
        char* token2 = strtok_r(token1, " ", &saveptr2);
        while (token2 && cmd->argc < MAX_ARGS-1) {
            if (strcmp(token2, "&") == 0) cmd->background = 1;
            else cmd->args[cmd->argc++] = token2;
            token2 = strtok_r(NULL, " ", &saveptr2);
        }
        cmd->args[cmd->argc] = NULL;

        token1 = strtok_r(NULL, ";", &saveptr1);
        num_commands++;
    }
    return num_commands;
}

int handle_builtin(char** arglist) {
    if (!arglist || !arglist[0]) return 0;

    if (strcmp(arglist[0], "cd") == 0) {
        if (!arglist[1]) fprintf(stderr, "cd: missing argument\n");
        else if (chdir(arglist[1]) != 0) perror("cd failed");
        return 1;
    }

    if (strcmp(arglist[0], "exit") == 0) exit(0);

    if (strcmp(arglist[0], "help") == 0) {
        printf("Built-in commands:\n");
        printf("cd <directory> - Change directory\n");
        printf("exit - Exit the shell\n");
        printf("help - Show this help message\n");
        printf("history - Show command history\n");
        printf("jobs - Show background jobs\n");
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

int execute(Command* cmd) {
    if (!cmd || !cmd->args[0]) return 0;
    if (handle_builtin(cmd->args)) return 1;

    pid_t cpid = fork();
    if (cpid < 0) { perror("fork failed"); exit(1); }

    if (cpid == 0) { // child
        execvp(cmd->args[0], cmd->args);
        perror("Command not found");
        exit(1);
    } else { // parent
        if (!cmd->background) waitpid(cpid, NULL, 0);
        else add_job(cpid, cmd->args[0]);
    }
    return 0;
}

void execute_parsed_commands(Command* commands, int num_commands) {
    for (int i = 0; i < num_commands; i++) execute(&commands[i]);
}

void free_commands(Command* commands, int count) {
    // nothing dynamically allocated inside commands struct
}

