#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void display_prompt() { printf("> "); }

char *get_input(char buf[], int buf_size) {
  char *input = fgets(buf, buf_size, stdin);

  if (!input) {
    // CTRL+D was entered causing EOF to be read which returns NULL
    return NULL;
  }

  const int input_size = strlen(input);
  if (input_size > 0) {
    // remove the newline from the end of the string
    input[input_size - 1] = '\0';
  }
  return input;
}

char *tokenize_input(char buf[]) {
  const char *delims = " \t>|<&;";
  char *token = strtok(buf, delims);
  printf("tok: \"%s\"\n", token);
  while (token) {
    token = strtok(NULL, delims);
    printf("tok: \"%s\"\n", token);
  }

  return buf;
}

int run() {
  // TODO: find the user home directory from the environment
  // TODO: set current working directory to the user home directory
  // TODO: save the current path
  // TODO: load history
  // TODO: load aliases

  // FIXME: what do we do when the command is longer than 510 + \n + \0?
  char prompt_buf[512] = {0};

  bool run = true;
  while (run) {
    // DONE: display the prompt
    display_prompt();

    // DONE: read
    char *input = get_input(prompt_buf, sizeof(prompt_buf));

    // handle internal command
    bool exit = input == NULL || strcmp("exit", input) == 0;
    if (exit)
      break;

    process_internal_commands(input);

    // TODO: parse the user input
    tokenize_input(input);

    // TODO: execute command
  }
  // TODO: save history
  // TODO: save aliases
  // TODO: restore the original path
  return 0;
}
