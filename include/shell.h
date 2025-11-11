#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SHELL_PROMPT "FCIT> "
#define INPUT_BUFFER_SIZE 1024
#define MAX_ARGS 64
#define MAX_BLOCK_LINES 20

// If-then-else block structure
typedef struct {
    char* condition;
    char* then_commands[MAX_BLOCK_LINES];
    char* else_commands[MAX_BLOCK_LINES];
    int then_count;
    int else_count;
    int in_if_block;
    int in_then_block;
    int in_else_block;
} IfBlock;

// Function declarations
char** tokenize(char* line);
void execute_command(char** args);
int handle_builtin(char** arglist);
char* read_cmd(void);
void init_if_block(IfBlock* if_block);
int parse_if_statement(char* line, IfBlock* if_block);
void execute_if_block(IfBlock* if_block);
char* read_cmd_if(void);

#endif
