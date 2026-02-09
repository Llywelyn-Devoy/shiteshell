#include "history.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shared.h"
#define HIST_SIZE 20


void create_history(struct history_arr *history) {
    history->count = 0;
    for (int i = 0; i < HIST_SIZE; i++)
        history->commands[i] = NULL;
}

void add_command_to_history(struct history_arr* history, const char *command) {
    int idx = history->count % HIST_SIZE;   //find the index
    free(history->commands[idx]);   //free if old command exists in this index
    history->commands[idx] = strdup(command);   //allocate new memory for command
    history->count++;
}

void print_commands(struct history_arr* history) {
    int total = history->count < HIST_SIZE ? history->count : HIST_SIZE; //find number of commands to print
    int start_num = history->count < HIST_SIZE ? 1 : history->count - HIST_SIZE + 1; //find oldest command
    
    for (int i = 0; i < total; i++) {
        int idx = (history->count - total + i) % HIST_SIZE; //find index of command 
        printf("%d %s\n", start_num + i, history->commands[idx]);   //print command number and command
    }
}

const char *last_command(struct history_arr *history) {
    if (history->count == 0)    //if history is empty return null
        return NULL;
    //last command is always at index count - 1 % 20
    return history->commands[(history->count - 1) % HIST_SIZE]; 
}

 //logic for commands starting with !
char *load_history_command(struct history_arr *history, char *dest_buf, const char *input_buf)
{
    //pointer sanity check 
    if (!history || !dest_buf || !input_buf)
        return NULL;

    //pointer for found command
    const char *cmd = NULL;

    // !! - return last command
    if (strcmp(input_buf, "!!") == 0) {
        cmd = last_command(history);
        if (!cmd) {
            printf("history is empty.\n");
            return NULL;
        }
    }
    // !N - command with number N
    else if (input_buf[0] == '!' && input_buf[1] != '-') {
        int n = atoi(input_buf + 1); //convert input string to int
        //making sure that command index is within history count
        if (n <= 0 || n > history->count) {
            printf("No such command in history.\n");
            return NULL;
        }
        int idx = (n - 1) % HIST_SIZE; //find right index
        if (history->count > HIST_SIZE) {
            int oldest_cmd_num = history->count - HIST_SIZE + 1;
            if (n < oldest_cmd_num) {   //this means command was overwritten
                printf("No such command in history.\n");
                return NULL;
            }
            idx = (n - 1) % HIST_SIZE;
        }
        cmd = history->commands[idx];
    }
    // !-N - command with number history->count - N
    else if (strncmp(input_buf, "!-", 2) == 0) {
        int offset = atoi(input_buf + 2); //string input to int
        if (offset <= 0 || offset > history->count) { // make sure input is within history
            printf("No such command in history.\n");
            return NULL;
        }
        int target = history->count - offset; //find target command number
        if (target < 0) {
            printf("No such command in history.\n");
            return NULL;
        }
        cmd = history->commands[target % HIST_SIZE];
    }
    else {
        return NULL;
    }

    if (!cmd) {
        printf("No such command in history.\n");
        return NULL;
    }

    strncpy(dest_buf, cmd, MAX_COMMAND_SIZE - 1); //copy command to destination buffer
    dest_buf[MAX_COMMAND_SIZE - 1] = '\0';  //null terminator
    return dest_buf;
}

