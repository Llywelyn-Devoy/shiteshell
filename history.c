#define _POSIX_C_SOURCE 200809L
#include "history.h"
#include "shared.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define HIST_SIZE 20

void create_history(struct history_arr *history) {
  history->count = 0;
  for (int i = 0; i < HIST_SIZE; i++)
    history->commands[i] = NULL;
  history->overflow = 0;
}

void add_command_to_history(struct history_arr *history, const char *command) {
  if (history->count >= HIST_SIZE - 1) {
    history->overflow = 1;
  }
  free(history->commands[history->count]);
  history->commands[history->count] = strdup(command);
  history->count = (history->count + 1) % HIST_SIZE;
}

void print_commands(struct history_arr *history) {
  if (history->overflow) {
    for (int i = history->count; i < HIST_SIZE + history->count; i++) {
      if (i >= history->count)
        printf("%d %s\n", i - history->count + 1,
               history->commands[i % HIST_SIZE]);
      else
        printf("%d %s\n", i + history->count + 1,
               history->commands[i % HIST_SIZE]);
    }
  } else {
    for (int i = 0; i < history->count; i++) {
      printf("%d %s\n", i + 1, history->commands[i]);
    }
  }
}

const char *last_command(struct history_arr *history) {
  return history->commands[(history->count - 1) % HIST_SIZE];
}

// logic for commands starting with !
char *load_history_command(struct history_arr *history, char *dest_buf,
                           const char *input_buf) {
  if (!history || !dest_buf || !input_buf)
    return NULL;

  const char *cmd = NULL;

  // !! - return last command
  if (strcmp(input_buf, "!!") == 0) {
    if (history->count == 0 && !history->overflow) {
      printf("history is empty.\n");
      return NULL;
    }
    int last_idx = (history->count - 1 + HIST_SIZE) % HIST_SIZE;
    cmd = history->commands[last_idx];
  }
  // !N - command with number N (1-based indexing)
  else if (input_buf[0] == '!' && input_buf[1] != '-') {
    int n = atoi(input_buf + 1);
    int total_commands = history->overflow ? HIST_SIZE : history->count;

    if (n <= 0 || n > total_commands) {
      printf("No such command in history.\n");
      return NULL;
    }

    // Map user-facing number to circular buffer index
    int idx;
    if (history->overflow) {
      idx = (history->count + n - 1) % HIST_SIZE;
    } else {
      idx = n - 1;
    }

    cmd = history->commands[idx];
  }
  // !-N - N commands back from last
  else if (strncmp(input_buf, "!-", 2) == 0) {
    int offset = atoi(input_buf + 2);
    int total_commands = history->overflow ? HIST_SIZE : history->count;

    if (offset <= 0 || offset > total_commands) {
      printf("No such command in history.\n");
      return NULL;
    }

    int last_idx = (history->count - 1 + HIST_SIZE) % HIST_SIZE;
    int idx = (last_idx - offset + 1 + HIST_SIZE) % HIST_SIZE;

    cmd = history->commands[idx];
  } else {
    return NULL;
  }

  if (!cmd) {
    printf("No such command in history.\n");
    return NULL;
  }

  strncpy(dest_buf, cmd, MAX_COMMAND_SIZE - 1);
  dest_buf[MAX_COMMAND_SIZE - 1] = '\0';
  return dest_buf;
}

// savig history to file
void save_history(struct history_arr *history) {
  if (!history)
    return;

  const char *home = getenv("HOME");
  if (!home)
    return;

  char path[MAX_COMMAND_SIZE];
  snprintf(path, sizeof(path), "%s/.hist_list", home);

  FILE *file = fopen(path, "w");
  if (!file) {
    perror("Failed to save history");
    return;
  }

  int total_commands = history->overflow ? HIST_SIZE : history->count;

  for (int i = 0; i < total_commands; i++) {
    int idx;

    if (history->overflow) {
      idx = (history->count + i) % HIST_SIZE;
    } else {
      idx = i;
    }

    if (history->commands[idx]) {
      fprintf(file, "%d %s\n", i + 1, history->commands[idx]);
    }
  }

  fclose(file);
}

// loading history from .hist_list
void load_history(struct history_arr *history) {
  if (!history)
    return;

  const char *home = getenv("HOME");
  if (!home)
    return;

  char path[MAX_COMMAND_SIZE];
  snprintf(path, sizeof(path), "%s/.hist_list", home);

  FILE *file = fopen(path, "r");
  // file does not exist
  if (!file) {
    return;
  }

  char line[MAX_COMMAND_SIZE];

  while (fgets(line, sizeof(line), file)) {
    char *newline = strchr(line, '\n');
    if (newline)
      *newline = '\0';

    // Skip number
    char *space = strchr(line, ' ');
    if (!space)
      continue;

    char *command = space + 1;

    add_command_to_history(history, command);
  }

  fclose(file);
}
