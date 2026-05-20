#include "io.h"
#include "shared.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define HISTORY_SIZE 20
#define HISTORY_FILENAME ".shiteshell_history"

typedef struct {
    int capacity;
    char **cmds;
    int start;
    int end;
} History;

char *history_get_filepath();

void history_load(History *history);

int history_count(const History *history);

int history_is_full(const History *history);

char *history_pop_front(History *history);

/*
 * Appends the provided command to the history if there's space, otherwise
 * replaces the oldest command. Returns the oldest command replaced or NULL.
 * */
char *history_push_back(History *history, char *cmd);

char *history_get(const History *history, int index);

char *history_peek_back(const History *history);

void history_print(const History *history);

void history_free(History *history);

bool history_save(const History *history);
