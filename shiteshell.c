#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define STR_EQ(a, b) (0 == strcmp(a, b))
#define INPUT_BUF_SIZE 512

void display_prompt() { printf("> "); }

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

  const char *delims = " \t>|<&;";
  char *token = strtok(buf, delims);
  int token_idx = 0;
  while (token) {
    assert(token_idx < INPUT_BUF_SIZE); // sanity check
    token_buf[token_idx++] = token;
    token = strtok(NULL, delims);
  }

  return token_buf;
}

/**
 * Run the shell.
 * */
int run() {
  // TODO: find the user home directory from the environment
  // TODO: set current working directory to the user home directory
  // TODO: save the current path
  // TODO: load history
  // TODO: load aliases

  char input_buf[INPUT_BUF_SIZE] = {0};

  // NOTE: we set the token_buf to the size of the input buf since it guarantees
  // we'll never run out of space in the token buffer.
  char *token_buf[INPUT_BUF_SIZE] = {0};

  while (true) {
    // DONE: display the prompt
    display_prompt();

    // DONE: read
    char *input = get_input(input_buf, sizeof(input_buf));

    // DONE: handle internal command
    bool exit = input == NULL || STR_EQ("exit", input);
    if (exit)
      break;

    // DONE: parse the user input
    char **tokens = tokenize_input(input, token_buf);
    for (int i = 0;; i++) {
      if (tokens[i] == NULL)
        break;
      printf("%s\n", tokens[i]);
    }

    // TODO: execute command
  }
  // TODO: save history
  // TODO: save aliases
  // TODO: restore the original path
  return 0;
}
