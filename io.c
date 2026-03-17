#include "shared.h"
#include <stdbool.h>
#include <stdio.h>
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
