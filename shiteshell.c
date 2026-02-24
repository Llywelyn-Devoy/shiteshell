#define _POSIX_C_SOURCE 200809L
#include "commands.h"
#include "history.h"
#include "io.h"
#include "shared.h"
#include <assert.h>
#include <errno.h> // errno
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> // pid_t datatype
#include <sys/wait.h>  // wait
#include <unistd.h>    // fork, execl

#define PATH_LIMIT 4096 // TODO: find out the path limit on linux
// The maximum path length is 4096 characters.
#define Max_alias 10

#define ALIAS_COMMAND_NAME "alias"
#define UNALIAS_COMMAND_NAME "unalias"

typedef struct {
  char name[64];
  char command[MAX_COMMAND_SIZE];
} Alias;

Alias aliases[Max_alias];
int no_alais = 0;

void display_prompt() { printf("> "); }

/**
 * Reads the user input from stdin and writes into the supplied buffer
 *
 * @param buf the buffer to write into
 * @param buf_size size, (elements) of the buf
 * @return the buf or NULL if CTRL+D was pressed
 */
char *get_input(char buf[], int buf_size) {
  char *input = fgets(buf, buf_size, stdin);

  // CTRL+D causes EOF which fgets returns as NULL
  if (!input) {
    return NULL;
  }

  const int input_size = strlen(input);
  if (input_size == 0) {
    return input;
  }

  char *last_char_ptr = &input[input_size - 1];
  if (*last_char_ptr == '\n') {
    // remove the newline
    *last_char_ptr = '\0';
  } else { // since no newline present the input is larger than the buffer size,
           // gracefully handle this
    fprintf(stderr,
            "ERROR: We don't support inputs larger than %d character(s)!\n",
            buf_size - 2 // for '\n' and '\0'
    );
    clear_stdin();   // handle the overflow
    input[0] = '\0'; // "clear" the input
  }

  return input;
}

/**
 * Clears the supplied null-terminated token buffer
 *
 * @param token_buf[] null-terminated token buffer
 */
void clear_token_buf(char *token_buf[]) {
  for (int i = 0;; i++) {
    if (token_buf[i] == NULL)
      break;
    else
      token_buf[i] = NULL;
  }
}

/**
 * Tokenizes the user input by the following characters  " \t>|<&;", modifying
 * the supplied input buffer and writing the token string slices into the
 * supplied null-terminated token buf.
 *
 * @param buf[] the user input, (modifies)
 * @param token_buf[] supplied null-terminated array of string to write into
 * @return the supplied token_buf
 * */
char **tokenize_input(char buf[], char *token_buf[]) {
  clear_token_buf(token_buf);

  /** TODO: allow escaping of delims when inside of double quotes, (or single
   * quotes maybe?). This is so the following will produce expected results:
   * `echo "hello; world"`
   * Currently produces: "hello world"
   * Should produce: "hello; world"
   * */
  const char *delims = " \t>|<&;";
  char *token = strtok(buf, delims);
  int token_idx = 0;
  while (token) {
    assert(token_idx < MAX_COMMAND_SIZE); // sanity check
    token_buf[token_idx++] = token;
    token = strtok(NULL, delims);
  }

  return token_buf;
}

void exec(const char *filename, char *const argv[]) {
  assert(filename);
  assert(argv);
  pid_t p = fork();
  if (p > 0) {         // parent
    wait(NULL);        // wait for all child processes
  } else if (p == 0) { // child
    // exec including sysenv PATH
    execvp(filename, argv);
    // for full list of errors look at `man execve`
    switch (errno) {   // TODO: handle the errors betterer.
    case EACCES:       // permission
    case ENAMETOOLONG: // pathname too long
    case ENOENT:       // pathname does not exist
    case ENOEXEC:      // cannot execute
    case ETXTBSY:      // executable is being written to
    default:
      // NOTE: for now just print out the error message, but in future
      // we might want to handle each of the above errors individually.
      fprintf(stderr, "ERROR: %s\n", strerror(errno));
      break;
    }
    // NOTE: make sure to end/unalive/kill the child process
    exit(errno);
  } else {
    // `man fork` to find out more about these errors
    fprintf(stderr, "ERROR: Failed to fork current process!\n");
    fprintf(stderr, "\t%s", strerror(errno));
  }
}

int wheres_my_alias(const char *name) {
  for (int i = 0; i < no_alais; i++) {
    if (strcmp(aliases[i].name, name) == 0)
      return i;
  }
  return -1;
}

bool is_there_an_alias(const char *input, int *alias_idx) {
  char first[64] = {0};
  sscanf(input, "%63s", first);

  int id = wheres_my_alias(first);
  if (id >= 0) {
    *alias_idx = id;
    return true;
  }
  return false;
}

void resolve_alias(const char *input, char *output) {
  char temp[MAX_COMMAND_SIZE];
  strcpy(temp, input);
  bool expanded[Max_alias] = {0};

  while (true) {
    char work[MAX_COMMAND_SIZE];
    strcpy(work, temp);

    char *tokens[MAX_COMMAND_SIZE] = {0};
    tokenize_input(work, tokens);

    char next[MAX_COMMAND_SIZE] = {0};
    bool changed = false;

    for (int i = 0; tokens[i]; i++) {
      int id = wheres_my_alias(tokens[i]);

      if (id >= 0 && !expanded[id]) {
        strcat(next, aliases[id].command);
        changed = true;
        expanded[id] = true;
      } else {
        strcat(next, tokens[i]);
      }

      if (tokens[i + 1])
        strcat(next, " ");
    }

    if (!changed){
      break;
    }
    

    strcpy(temp, next);
  }

  strcpy(output, temp);
}

void alias_commands(char *const tokens[]) {
  const char *cmd_name = tokens[0];
  const char *alias_name = tokens[1];
  char *const *rest = &tokens[2];

  if (!alias_name) {
    for (int i = 0; i < no_alais; i++) {
      printf("%s ='%s'\n", aliases[i].name, aliases[i].command);
    }
    return;
  }

  if (STR_EQ(ALIAS_COMMAND_NAME, alias_name) ||
      STR_EQ(UNALIAS_COMMAND_NAME, alias_name)) {
    fprintf(stderr, "just dont do that\n");
    return;
  }

  if (!rest[0]) {
    fprintf(stderr, "alias with no command\n");
    return;
  }

  char command[MAX_COMMAND_SIZE] = {0};
  for (int i = 2; tokens[i]; i++) {
    strcat(command, tokens[i]);
    if (tokens[i + 1])
      strcat(command, " ");
  }

  int alias_idx = wheres_my_alias(alias_name);

  if (alias_idx >= 0) {
    printf("alias with same name as previous alias removing previous alias\n");
    strcpy(aliases[alias_idx].command, command);
  } else {
    if (no_alais >= Max_alias) {
      fprintf(stderr, "wooh there bucko stop renaming stuff 10 max\n");
      return;
    }
    strcpy(aliases[no_alais].name, alias_name);
    strcpy(aliases[no_alais].command, command);
    no_alais++;
  }
}

void unalias_command(char *const tokens[]) {
  if (!tokens[1]) {
    fprintf(stderr, "why are you unaliasing nothing\n");
    return;
  }

  int alias_idx = wheres_my_alias(tokens[1]);
  if (alias_idx < 0) {
    fprintf(stderr, "sorry to tell you but that alias aint real\n");
    return;
  }

  for (int i = alias_idx; i < no_alais - 1; i++) {
    aliases[i] = aliases[i + 1];
  }
  no_alais--;
}

void execute_command(char *const tokens[]) {
  assert(*tokens);

  if (try_run_internal_command(tokens[0], tokens))
    return;

  exec(tokens[0], tokens);
}

/**
 * Run the shell.
 * */
int run() {
  // DONE: save the current path
  char *prelude_path = getenv(ENV_PATH);
  assert(prelude_path);

  // DONE: find the user home directory from the environment
  // DONE: set current working directory to the user home directory
  change_directory("~");

  // DONE: load history
  struct history_arr history;
  create_history(&history);
  load_history(&history);

  // TODO: load aliases

  char input_buf[MAX_COMMAND_SIZE] = {0};

  // NOTE: we set the token_buf to the size of the input buf since it guarantees
  // we'll never run out of space in the token buffer.
  char *token_buf[MAX_COMMAND_SIZE] = {0};

  char cooked_buf[MAX_COMMAND_SIZE] = {0};

  while (true) {
    // DONE: display the prompt
    display_prompt();

    // DONE: read
    // TODO: handle CTRL+C for cancelling child process
    char *input = get_input(input_buf, sizeof(input_buf));

    // NOTE: special case for CTRL+D
    if (!input)
      break;

    // Skip empty input
    if (input[0] == '\0')
      continue;

    resolve_alias(input_buf, cooked_buf);

    char *command_to_process = cooked_buf;
    char history_expanded_buf[MAX_COMMAND_SIZE] = {
        0}; // buffer to hold comand found in history
    char *tokens[MAX_COMMAND_SIZE] = {0};

    // load history command if input is history invoke
    if (input[0] == '!') {
      char *loaded =
          load_history_command(&history, history_expanded_buf, input);
      if (!loaded) {
        continue;
      }

      printf("%s\n", loaded);
      command_to_process = loaded; // update pointer to point to the history
                                   // command instead of the ! string.
    }
    // command is not history invoke
    else {
      add_command_to_history(&history, input);
    }

    tokenize_input(command_to_process, tokens);
    if (!tokens[0])
      continue;

    // print whole history when input is "history"
    if (STR_EQ("history", tokens[0]) && !tokens[1]) {
      print_commands(&history);
      continue;
    }

    // TODO: resolve aliases

    // NOTE: special case handle internal command exit
    if (STR_EQ("exit", tokens[0]) && !tokens[1])
      break;

    // DONE: execute command
    execute_command(tokens);
  }

  save_history(&history);
  // TODO: save aliases
  // DONE: restore the original path
  printf("\n");
  printf("Restoring original path...\n");
  int err = setenv(ENV_PATH, prelude_path, true);
  assert(!err);
  printf("Path restored to:\n");
  printf("\"%s\"\n", prelude_path);

  printf("Exiting normally...\n");

  for (int i = 0; i < HIST_SIZE; i++) {
    if (history.commands[i])
      free(history.commands[i]);
  }

  return 0;
}
