#include <stdbool.h>
#include <string.h>

#define DEFER(cleanup_func)                                                    \
    for (int __done__ = 0; !__done__; (cleanup_func), __done__ = 1)
#define STR_EQ(a, b) (0 == strcmp(a, b))
#define STR_STARTSWITH(str, prefix) (0 == strncmp(str, prefix, strlen(prefix)))
#define ARRAY_LEN(arr) ((int)(sizeof(arr) / sizeof(arr[0])))
#define ENV_PATH "PATH"
#define ENV_HOME "HOME"

#define MAX_COMMAND_SIZE 512

/*
 * Free and set to
 * null.
 *
 * @param ptr, the
 * pointer to free and
 * set to null.
 * */
void shite_free(void **ptr);

/*
 * Changes directory, handling the errors and printing them to stderr.
 * Resolves '~' to the user home directory.
 *
 * @param new_path, path to change to.
 * @return true if successfully changed path, otherwise false.
 * */
bool change_directory(const char *new_path);
