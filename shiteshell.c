#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_BUF_SIZE 512

void display_prompt() { printf("> "); }

void clear_till_EOF() {
  while (true) {
    int c = fgetc(stdin);
    if (c == '\n' || c == EOF)
      // we've cleared the input buffer
      return;
  }
}
char *get_input(char buf[], int buf_size) {
  char *input = fgets(buf, buf_size, stdin);

  if (!input) {
    // CTRL+D was entered causing EOF to be read which returns NULL
    return NULL;
  }

  const int input_size = strlen(input);
  if (input_size == 0) {
    return "";
  }

  char *last_char = &input[input_size - 1];

  if (*last_char == '\n') {
    // remove the newline from the end of the string
    *last_char = '\0';
  } else {
    printf("ERROR: We don't support inputs larger than %d characters!\n",
           INPUT_BUF_SIZE - 2 // for '\n' and '\0'
    );
    clear_till_EOF();
    input = "";
  }

  return input;
}

char *token_buf[INPUT_BUF_SIZE] = {0};

void clear_token_buf() {
  for (int i = 0; i < INPUT_BUF_SIZE; i++) {
    token_buf[i] = NULL;
  }
}

/*Tokenizes the user inputs by the following characters  " \t>|<&;"
 * and returns a null terminated array of string slices from the input buffer.
 *
 * */
char **tokenize_input(char buf[]) {
  clear_token_buf();

  const char *delims = " \t>|<&;";
  char *token = strtok(buf, delims);
  int token_idx = 0;
  while (token) {
    assert(token_idx < INPUT_BUF_SIZE);
    if (token)
      token_buf[token_idx++] = token;
    token = strtok(NULL, delims);
  }

  return token_buf;
}

char input_buf[INPUT_BUF_SIZE] = {0};

int run() {
  // TODO: find the user home directory from the environment
  // TODO: set current working directory to the user home directory
  // TODO: save the current path
  // TODO: load history
  // TODO: load aliases

  // FIXME: what do we do when the command is longer than 510 + \n + \0?
  bool run = true;
  while (run) {
    // DONE: display the prompt
    display_prompt();

    // DONE: read
    char *input = get_input(input_buf, sizeof(input_buf));

    // DONE: handle internal command
    bool exit = input == NULL || strcmp("exit", input) == 0;
    if (exit)
      break;

    // DONE: parse the user input
    char **tokens = tokenize_input(input);
	for(int i = 0;; i++) {
		if(tokens[i] == NULL)
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

// dump, (don't look)
/*
char *trim_by_delims(char *str, char *delims) {
        // find the "true" start of the
        //
        // ";;;hello;;;\0"
        //     ^   ^

        char *true_str_cursor = str;
        char *cursor = str;
        while(*cursor) {
                // check if cursor is one of the delims
                bool is_delim = false;
                char *delim_cursor = delims;
                while(*delim_cursor) {
                        if(*cursor == *delim_cursor) {
                                is_delim = true;
                                break;
                        }
                }
                if(is_delim) {
                        *cursor = '\0';
                        cursor++;
                        continue;
                }

                if(!str_start) {
                        str_start = cursor;
                }
        }

}
*/
