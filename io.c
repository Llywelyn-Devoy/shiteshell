#include "shared.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> // for filestystem stats

bool directory_exists(const char *dir_path) {
    struct stat dir_stat = {0};
    if (0 == stat(dir_path, &dir_stat))
        return S_ISDIR(dir_stat.st_mode);
    else
        return false;
}

/**
 * Clear the stdin buffer
 */
void clear_stdin() {
    while (true) {
        int c = fgetc(stdin);
        if (c == '\n' || c == EOF)
            // we've cleared the input buffer
            return;
    }
}

// Source - https://stackoverflow.com/a/70708991
// Posted by Mike Siomkin, modified by community. See post 'Timeline' for change
// history Retrieved 2026-05-08, License - CC BY-SA 4.0
int count_lines(FILE *file) {
#define COUNT_LINES_BUF_SIZE 65536
    char buf[COUNT_LINES_BUF_SIZE];
    int counter = 0;
    for (;;) {
        size_t res = fread(buf, 1, COUNT_LINES_BUF_SIZE, file);
        if (ferror(file))
            return -1;

        for (size_t i = 0; i < res; i++)
            if (buf[i] == '\n')
                counter++;

        if (feof(file))
            break;
    }

    return counter;
#undef COUNT_LINES_BUF_SIZE
}

bool read_lines(const char *filepath, char ***lines_out, int *line_count_out) {
    FILE *fp = fopen(filepath, "r");
    if (!fp)
        return false;

    *line_count_out = count_lines(fp);
    rewind(fp);
    if (*line_count_out < 0)
        return false;

    char **lines = malloc(*line_count_out * sizeof(lines[0]));
    assert(lines);

    int lines_idx = 0;
    char buf[MAX_COMMAND_SIZE + 1] = {0}; // NOTE: +1 for newline
    while (fgets(buf, sizeof(buf), fp)) {
        char *newline = strrchr(buf, '\n');
        if (!newline) {
            // BUG: returning without cleaning up
            // invalid file as lines too big
            return false;
        } else {
            *newline = '\0';
        }
        lines[lines_idx++] = strdup(buf);
    }
    assert(*line_count_out == lines_idx);
    fclose(fp);

    *lines_out = lines;
    return true;
}
