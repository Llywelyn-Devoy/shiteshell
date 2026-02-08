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
    int idx = history->count % HIST_SIZE;
    free(history->commands[idx]);
    history->commands[idx] = strdup(command);
    history->count++;
}

void print_commands(struct history_arr* history) {
    int total = history->count < HIST_SIZE ? history->count : HIST_SIZE;
    int start_num = history->count < HIST_SIZE ? 1 : history->count - HIST_SIZE + 1;
    
    for (int i = 0; i < total; i++) {
        int idx = (history->count - total + i) % HIST_SIZE;
        printf("%d %s\n", start_num + i, history->commands[idx]);
    }
}

const char *last_command(struct history_arr *history) {
    if (history->count == 0)
        return NULL;
    return history->commands[(history->count - 1) % HIST_SIZE];
}

 //print last command "!!" or command number "!no" "!-no"
char *load_history_command(struct history_arr *history, char *dest_buf, const char *input_buf)
{
    if (!history || !dest_buf || !input_buf)
        return NULL;

    const char *cmd = NULL;

    // !!
    if (strcmp(input_buf, "!!") == 0) {
        cmd = last_command(history);
        if (!cmd) {
            printf("No commands in history.\n");
            return NULL;
        }
    }
    // !N
    else if (input_buf[0] == '!' && input_buf[1] != '-') {
        int n = atoi(input_buf + 1);
        if (n <= 0 || n > history->count) {
            printf("No such command in history.\n");
            return NULL;
        }
        int idx = (n - 1) % HIST_SIZE; //find right index
        if (history->count > HIST_SIZE) {
            int oldest_cmd_num = history->count - HIST_SIZE + 1;
            if (n < oldest_cmd_num) {
                printf("No such command in history.\n");
                return NULL;
            }
            idx = (n - 1) % HIST_SIZE;
        }
        cmd = history->commands[idx];
    }
    // !-N
    else if (strncmp(input_buf, "!-", 2) == 0) {
        int offset = atoi(input_buf + 2);
        if (offset <= 0 || offset > history->count) {
            printf("No such command in history.\n");
            return NULL;
        }
        int target = history->count - offset;
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

    strncpy(dest_buf, cmd, MAX_COMMAND_SIZE - 1);
    dest_buf[MAX_COMMAND_SIZE - 1] = '\0';
    return dest_buf;
}

