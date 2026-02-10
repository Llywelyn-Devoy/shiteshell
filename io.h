#include <stdbool.h>
#include <string.h>
#include <sys/stat.h> // for filestystem stats

bool directory_exists(const char *dir_path);

void load_file(char *path_to_file);
/**
 * Clear the stdin buffer
 */
void clear_stdin();
