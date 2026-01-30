#include <stdbool.h>
#include <string.h>
#include <sys/stat.h> // for filestystem stats

bool directory_exists(const char *dir_path);

/**
 * Clear the stdin buffer
 */
void clear_stdin();
