#define _POSIX_C_SOURCE 200809L
#include "io.h"
#include "shared.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Checks if a colon-separated path list is valid. The list is valid if
 * all of the paths within it are valid directories.
 *
 * @param path, colon-separated list of paths
 * @param path_len, the length of the path str
 * @return `true` if the path list is valid, `false` otherwise.
 * */
bool is_valid_path(const char *path, int path_len) {
  // duplicate the path as to not overwrite it
  char path_dup[path_len + 1];
  memcpy(path_dup, path, sizeof(path_dup));
  char delim[] = ":";
  char *dir_path = strtok(path_dup, delim);
  while (dir_path) {
    if (!directory_exists(dir_path))
      // TODO: return this invalid path, so that we can be more specific to the
      // user about the issue.
      return false;
    dir_path = strtok(NULL, delim);
  }
  return true;
}

void cmd_getpath(char *const argv[]) {
  const char *const name = argv[0];
  if (argv[1]) {
    fprintf(stderr, "ERROR: `%s` expects no arguments.\n", name);
    return;
  }

  const char *path = getenv(ENV_PATH);
  assert(path);
  printf("%s\n", path);
  return;
}

void cmd_setpath(char *const argv[]) {
  const char *const name = argv[0];
  if (!argv[1] || argv[2]) {
    fprintf(stderr,
            "ERROR: Invalid arguments. "
            "`%s` expects one argument; a colon-separated list to set the path "
            "to.\n",
            name);
    return;
  }

  char *new_path = argv[1];
  if (!is_valid_path(new_path, strlen(new_path))) {
    fprintf(stderr, "ERROR: path provided is invalid. It must be "
                    "colon-separated and all paths within must be valid.\n");
    return;
  }

  int err = setenv(ENV_PATH, new_path, true);
  assert(!err);

  printf("\"%s\" has been set.\n", ENV_PATH);
  return;
}

void cmd_change_dir(char *const argv[]) {
  const char *name = argv[0];
  const char *path = argv[1];
  const char *null = argv[2];
  assert(name);
  if (null != NULL) {
    fprintf(stderr,
            "ERROR: invalid arguments."
            "`%s` requires one optional argument for the path to change to.\n",
            name);
    return;
  }

  if (path) {
    change_directory(path);
  } else {
    // default to user home
    change_directory("~");
  }
}
void alias_command(char *const tokens[]);
void unalias_command(char *const tokens[]);

typedef struct {
  char *name;
  void (*action)(char *const[]);
} Command;
Command INTERNAL_COMMANDS[] = {
    {"getpath", &cmd_getpath},    {"setpath", &cmd_setpath},
    {"cd", &cmd_change_dir},      {"alias", alias_command},
    {"unalias", unalias_command},
};

/**
 * Tries to run the specified internal command, returning whether or not it was
 * successful.
 *
 * @param name, the name of the internal command to try and run.
 * @param argv, the arguments, _including_the_internal_commmand name_ to pass to
 * the internal command.
 * @return true if successfully ran the internal command, otherwise false.
 * */
bool try_run_internal_command(char *const name, char *const argv[]) {
  assert(argv);
  for (int i = 0; i < ARRAY_LEN(INTERNAL_COMMANDS); i++) {
    const Command cmd = INTERNAL_COMMANDS[i];
    if (STR_EQ(name, cmd.name)) {
      cmd.action(argv);
      return true;
    }
  }
  return false;
}
