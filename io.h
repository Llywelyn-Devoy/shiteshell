#include <stdbool.h>
#include <string.h>

typedef struct MemMapT {
  size_t size;
  int fd;
  char *buf;
} MemMap;

typedef struct {
  char *str;
  char *curr_chr;
  bool done;
} LineIter;


bool directory_exists(const char *dir_path);

void load_file(char *path_to_file);
/**
 * Clear the stdin buffer
 */
void clear_stdin();

bool mmap_file(const char *filepath, MemMap *mm);

LineIter lineiter_create(char *str);

bool lineiter_next(LineIter *li, char **line, size_t *line_len);

bool read_lines(const char *filepath, char ***lines_out, int *line_count_out);
