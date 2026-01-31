#include <assert.h>
#include <errno.h> // errno
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void change_directory(const char *new_path) {
  int err = chdir(new_path);
  if (err) { // NOTE: we crash for now, maybe we just want to continue without
             // going to user home?
    if (errno == ENOENT) // invalid path
      fprintf(stderr,
              "ERROR: couldn't change directory, invalid path \"%s\"\n.",
              new_path);
    else
      fprintf(stderr, "ERROR: %s\n", strerror(errno));
    exit(errno);
  }
}
