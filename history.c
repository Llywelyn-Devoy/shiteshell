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

char *history_get_filepath() {
    const char *home = getenv("HOME");

    const int buf_size = snprintf(NULL, 0, "%s/%s", home, HISTORY_FILENAME);
    char *buf = malloc((buf_size+1) * sizeof(buf[0]));
    const int written =
        snprintf(buf, buf_size + 1, "%s/%s", home, HISTORY_FILENAME);
    assert(buf_size == written);
    return buf;
}

void history_load(History *history) {

    int line_count = 0;
    char **lines = NULL;
    const char *filepath = history_get_filepath();
    read_lines(filepath, &lines, &line_count);
    shite_free((void **)&filepath);

    if (line_count > HISTORY_SIZE) {
        perror("ERROR: invalid history save file at ~/.shiteshell_history.");
        exit(-1);
    }

    history->capacity = HISTORY_SIZE + 1;
    lines = realloc(lines, history->capacity * sizeof(lines[0]));
    assert(lines);

    history->cmds = lines;
    history->start = 0;
    history->end = line_count;
}

int history_count(const History *history) {
    const int delta = history->end - history->start;
    return (history->capacity + delta) % history->capacity;
}

int history_is_full(const History *history) {
    return history->capacity - 1 == history_count(history);
}

int _wrapping_add(int a, int b, int limit) { return (a + b) % limit; }

char *history_pop_front(History *history) {
    if (0 == history_count(history)) {
        return NULL;
    }
    char *popped = history->cmds[history->start];
    history->start = _wrapping_add(history->start, 1, history->capacity);
    return popped;
}

/*
 * Appends the provided command to the history if there's space, otherwise
 * replaces the oldest command.
 * */
void history_push_back(History *history, char *cmd) {
    if (history_is_full(history)) {
        const char *popped = history_pop_front(history);
        shite_free((void **)&popped);
    }

    history->cmds[history->end] = cmd;
    history->end = _wrapping_add(history->end, 1, history->capacity);
}

char *history_get(const History *history, const int index) {
    if (index < 0 || index >= history_count(history))
        return NULL;

    const int true_index =
        _wrapping_add(history->start, index, history->capacity);

    return history->cmds[true_index];
}

char *history_peek_back(const History *history) {
    return history_get(history, history_count(history) - 1);
}

void history_print(const History *history) {
    const int count = history_count(history);
    for (int i = 0; i < count; i++) {
        printf("%2d. %s\n", i + 1, history_get(history, i));
    }
}

void history_free(History *history) {
    const int count = history_count(history);
    for (int i = 0; i < count; i++) {
        free(history_get(history, i));
    }
    shite_free((void **)&history->cmds);
}

bool history_save(const History *history) {
    const char *filepath = history_get_filepath();
    FILE *fp = fopen(filepath, "w");
    shite_free((void **)&filepath);

    if (!fp)
        return false;

    const int count = history_count(history);
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s\n", history_get(history, i));
    }

    fclose(fp);
    return true;
}
