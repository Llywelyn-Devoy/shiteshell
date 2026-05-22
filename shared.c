#include "shared.h"
#include <assert.h>
#include <errno.h> // errno
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

inline int abs(int n) { return n < 0 ? -n : n; }

/*
 * Free and set to
 * null.
 *
 * @param ptr, the
 * pointer to free and
 * set to null.
 * */
void shite_free(void **ptr) {
    free(*ptr);
    *ptr = NULL;
}

/*
 * Changes directory, handling the errors and printing them to stderr.
 * Resolves '~' to the user home directory.
 *
 * @param new_path, path to change to.
 * */
void change_directory(const char *new_path) {
    assert(new_path);

    char scratch[PATH_MAX];

    const char *tilde = "~";
    if (STR_STARTSWITH(new_path, tilde)) {
        const char *trimmed = new_path + strlen(tilde);
        const char *home = getenv("HOME");
        strcpy(scratch, home);
        strcat(scratch, trimmed);
        new_path = scratch;
    }

    int err = chdir(new_path);
    if (err) {
        // `man chdir` for the errors
        switch (errno) {
        case EACCES:       // perms issue
        case ENAMETOOLONG: // path too long; no need to brag.
        case ENOTDIR:      // a path component is not a dir
        case ENOENT:       // path doesn't point to a dir
            fprintf(stderr, "ERROR: Invalid path '%s'. %s.\n", new_path,
                    strerror(errno));
            break;
        default:
            fprintf(stderr, "ERROR: %s.\n", strerror(errno));
            exit(errno);
            break;
        }
    }
}
