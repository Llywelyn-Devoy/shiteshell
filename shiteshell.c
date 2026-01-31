#include "commands.h"
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

#define PATH_LIMIT 256 // TODO: find out the path limit on linux

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

  // TODO: load history
  // TODO: load aliases

  char input_buf[MAX_COMMAND_SIZE] = {0};

  // NOTE: we set the token_buf to the size of the input buf since it guarantees
  // we'll never run out of space in the token buffer.
  char *token_buf[MAX_COMMAND_SIZE] = {0};

  while (true) {
    // DONE: display the prompt
    display_prompt();

    // DONE: read
    // TODO: handle CTRL+C for cancelling child process
    char *input = get_input(input_buf, sizeof(input_buf));

    // NOTE: special case for CTRL+D
    if (!input)
      break;

    // DONE: parse the user input
    char **tokens = tokenize_input(input, token_buf);
    if (!tokens[0])
      continue;

    // TODO: resolve aliases

    // NOTE: special case handle internal command exit
    if (STR_EQ("exit", tokens[0]) && !tokens[1])
      break;

    // TODO: execute command
    execute_command(tokens);
  }
  // TODO: save history
  // TODO: save aliases
  // DONE: restore the original path
  printf("\n");
  printf("Restoring original path...\n");
  int err = setenv(ENV_PATH, prelude_path, true);
  assert(!err);
  printf("Path restored to:\n");
  printf("\"%s\"\n", prelude_path);

  printf("Exiting normally...\n");
  return 0;
}
