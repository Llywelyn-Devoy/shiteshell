#include <assert.h>
#include <errno.h> // errno
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void change_directory(const char *new_path) {
  int err = chdir(new_path);
  if (err) {
    // `man chdir` for the errors
    switch (errno) {
    case EACCES:       // perms issue
    case ENAMETOOLONG: // path too long; no need to brag.
    case ENOTDIR:      // a path component is not a dir
    case ENOENT:       // path doesn't point to a dir
      fprintf(stderr, "ERROR: %s.\n", strerror(errno));
      break;
    default: // more obscure errors so we'll just crash. It's called shiteshell
             // for a reason after all.
      fprintf(stderr, "ERROR: %s.\n", strerror(errno));
      // TODO: how do we crash safely here? We need to save history and aliases
      // for instance. Worth asking the helper?
      exit(errno);
      break;
    }
  }
}
