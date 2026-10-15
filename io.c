#include "io.h"
#include "shared.h"
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h> // for filestystem stats
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

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

// Source - https://stackoverflow.com/a/70708991
// Posted by Mike Siomkin, modified by community. See post 'Timeline' for change
// history Retrieved 2026-05-08, License - CC BY-SA 4.0
int count_lines(FILE *file) {
#define COUNT_LINES_BUF_SIZE 65536
    char buf[COUNT_LINES_BUF_SIZE];
    int counter = 0;
    for (;;) {
        size_t res = fread(buf, 1, COUNT_LINES_BUF_SIZE, file);
        if (ferror(file))
            return -1;

        for (size_t i = 0; i < res; i++)
            if (buf[i] == '\n')
                counter++;

        if (feof(file))
            break;
    }

    return counter;
#undef COUNT_LINES_BUF_SIZE
}

// returns ~0 on error.
bool get_filesize_from_file_desc(int fd, size_t *size) {
    if (fd == -1) {
      return false;
    }
    struct stat s;
    if (fstat(fd, &s) == -1) {
      return false;
    }
    *size = s.st_size;
    return true;
}

bool mmap_file(const char *filepath, MemMap *mm) {
    if(!filepath)
      return false;

    int fd = open(filepath, O_RDONLY);
    if(fd == -1)
      return false;

    if(!get_filesize_from_file_desc(fd, &mm->size))
      return false;

    mm->buf = mmap(NULL, mm->size, PROT_READ, MAP_PRIVATE, fd, 0);
    if(mm->buf == MAP_FAILED)
      return false;

    return true;
}

size_t count_lines_buf(const char *buf, size_t buf_size) {
  // TODO: use strchr instead?
  size_t count = 0;
  for(size_t i = 0; i < buf_size; i++)
    if(buf[i] == '\n')
      count++;
  return count;
}

LineIter lineiter_create(char *str) {
  LineIter li = {
    .str = str,
    .curr_chr = str-1,
    .done = false,
  };
  return li;
}

bool lineiter_next(LineIter *li, char **line, size_t *line_len) {
  *line = NULL;
  *line_len = 0;

  if(li->done)
    return false;

  // SAFETY: we've checked if we're at the end of the string already
  (li->curr_chr)++;
  *line = li->curr_chr;
  while(true) {
    if(*li->curr_chr == '\n') {
      break;
    } else if(*li->curr_chr == '\0') {
      li->done = true;
    } else {
      li->curr_chr++;
      (*line_len)++;
    }
  }

  return true;
}


/**
 * Reads the lines from a file.
 *
 * @param filepath, path to the file to read from.
 * @param lines_out, out pointer to an array of strings. *This function takes
 *                   care of allocation of the array*.
 * @param line_count_out, pointer to a variable to store how many lines were
 *                        read.
 * @return `true` if the file was read successfully, false otherwise. An empty
 * or non-present file still count as success, and results in a 0 allocation.
 * */
bool read_lines(const char *filepath, char ***lines_out, int *line_count_out) {
    bool result = true;

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        if (errno == ENOENT) {
            // No such file
            *line_count_out = 0;
            // NOTE: see `man malloc` for behaviour when size is 0. On linux
            // this is safe.
            *lines_out = NULL;
            return true;
        } else {
            return false;
        }
    }

    *lines_out = NULL;
    *line_count_out = count_lines(fp);
    rewind(fp);
    if (*line_count_out < 0) {
        result = false;
        goto exit;
    }

    char **lines = malloc(*line_count_out * sizeof(lines[0]));
    assert(lines);

    int lines_idx = 0;
    char buf[MAX_COMMAND_SIZE + 1] = {0}; // NOTE: +1 for newline
    while (fgets(buf, sizeof(buf), fp)) {
        char *newline = strrchr(buf, '\n');
        if (!newline) {
            goto cleanup_lines;
        } else {
            *newline = '\0';
        }
        lines[lines_idx++] = strdup(buf);
    }
    assert(*line_count_out == lines_idx);

    *lines_out = lines;

exit:
    fclose(fp);

    return result;

cleanup_lines:
    for (int i = 0; i < lines_idx; i++)
        free(lines[i]);
    shite_free((void **)&lines);
    goto exit;
}


void main() {
  MemMap mm;
  if(!mmap_file("io.c", &mm)) {
    return;
  }

  printf("mmap ends with \\0?: ");
  if(mm.buf[mm.size] == '\0') {
    printf("yes\n");
  } else {
    printf("no\n");
  }

  LineIter li = lineiter_create(mm.buf);
  char *line = NULL;
  size_t line_len = 0;

  size_t line_no = 1;
  while(lineiter_next(&li, &line, &line_len)) {
    printf("\t");
    for(int pp = 0; pp < line_len; pp++)
      printf("%c", line[pp]);
    printf("\n");
    // printf("\t\"%s\"\n", line);
    break;
    line_no++;
  }

  return;
}


