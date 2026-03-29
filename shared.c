#include "shared.h"
#include <assert.h>
#include <errno.h> // errno
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
 * Resolve '~' at the start of a path.
 *
 * @param path, the path to resolve
 * @return an allocated string with '~' resolved, or null there's no
 * resolution of '~'.
 * */
char *resolve_tilde(const char *path) {
    if (!STR_STARTSWITH(path, "~")) {
        return NULL;
    }
    assert(strlen(path) > 0);
    const char *home_path = getenv(ENV_HOME);
    assert(home_path);
    assert(strlen(path) > 0);
    const char *rm_tilde = &path[1]; // SAFETY: we know strlen(path) > 0
    const int buf_size = strlen(home_path) + strlen(rm_tilde) + 1;
    char *buf = malloc(buf_size);
    assert(buf);

    strcpy(buf, home_path);
    strcat(buf, rm_tilde);

    return buf;
}

/*
 * Changes directory, handling the errors and printing them to stderr.
 * Resolves '~' to the user home directory.
 *
 * @param new_path, path to change to.
 * */
void change_directory(const char *new_path) {

    const char *resolved = resolve_tilde(new_path);
    // NOTE: we need to make sure we free the resolved path
    DEFER(shite_free((void **)&resolved)) {
        if (resolved)
            new_path = resolved;

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
}
