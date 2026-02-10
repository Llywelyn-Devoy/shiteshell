#include "shared.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h> // for filestystem stats

bool directory_exists(const char *dir_path) {
  struct stat dir_stat = {0};
  if (0 == stat(dir_path, &dir_stat))
    return dir_stat.st_mode & S_IFDIR;
  else
    return false;
}

void load_file(char *path_to_file) {
  FILE *fptr;
  char the_command[MAX_COMMAND_SIZE] = "";
  // Opening the file in read mode
  fptr = fopen(path_to_file, "r");

  // checking if the file is
  // opened successfully
  if (fptr == NULL) {
    printf("file.txt file failed to open.");
  } else {

    printf("The file is now opened.\n");

    // Read the data from the file
    // using fgets() method
    while (fgets(the_command, MAX_COMMAND_SIZE, fptr) != NULL) {

      // Print the data
      printf("%s", the_command);
    }

    // Closing the file using fclose()
    fclose(fptr);
  }
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
