#define _POSIX_C_SOURCE 200809L
#include "alias.h"
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

typedef struct {
    struct history_arr history;
} ShiteShellState;

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
    } else { // since no newline present the input is larger than the buffer
             // size, gracefully handle this
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

/**
 * Attempts to execute the specified program, with the supplied arguments.
 * Errors are printed to stderr.
 *
 * @param filename path to the file to execute
 * @param token_buf[] null terminated arguments assuming the first argument is
 * the path to the file
 * */
void exec(const char *filename, char *const argv[]) {
    assert(filename);
    assert(argv);
    pid_t p = fork();
    if (p > 0) {         // parent
        wait(NULL);      // wait for all child processes
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
            fprintf(stderr, "ERROR:");
            for (int i = 0; argv[i] != NULL; i++) {
                fprintf(stderr, " %s", argv[i]);
            }
            fprintf(stderr, ": %s\n", strerror(errno));
        }
        // NOTE: make sure to end/unalive/kill the child process
        exit(errno);
    } else {
        // `man fork` to find out more about these errors
        fprintf(stderr, "ERROR: Failed to fork current process!\n");
        fprintf(stderr, "\t%s", strerror(errno));
    }
}

/**
 * Executes a command. Checks the internal commands first.
 *
 * @param tokens[], null-terminated array of token strings.
 * */
void execute_command(char *const tokens[]) {
    if (!tokens || !tokens[0])
        return;

    if (try_run_internal_command(tokens[0], tokens))
        return;

    exec(tokens[0], tokens);
}

void print_banner() {
    printf("  _____ _     _ _        _____ _          _ _  \n"
           " / ____| |   (_) |      / ____| |        | | | \n"
           "| (___ | |__  _| |_ ___| (___ | |__   ___| | | \n"
           " \\___ \\| '_ \\| | __/ _ \\\\___ \\| '_ \\ / _ \\ | | \n"
           " ____) | | | | | ||  __/____) | | | |  __/ | |\n"
           "|_____/|_| |_|_|\\__\\___|_____/|_| |_|\\___|_|_| \n"
           "                      W0oco0W                  \n"
           "                     N0occcdK                  \n"
           "                 WN0kdlcllllkN                 \n"
           "              WX0kolcclooolcdX                 \n"
           "         WNKOxdlllooooodol::lk0NW              \n"
           "       WKxl:::cllloolllc:;;:clodOXW            \n"
           "      Xkc;;;:clcc:::::::cclooooooxKW           \n"
           "     Xd:;;;;;::ccclllloooooooooooldK           \n"
           "    W0c:coxkOOkdlccllllldk0000Oxlcl0W          \n"
           "     0oxKW NK00KKklcccoOKK00XWWN0do0W          \n"
           "  WXOdON WOc,'';dKkc:l0Oc,'';dX  Xxx0N         \n"
           " NOocl0  Xc.....'k0l;d0l.....,xW WOooxKW       \n"
           "Xxc::ckW Nx,....c0Oc;l0x;...'c0  Nkoood0       \n"
           "kc::::lkXWWKkdxOKOdlccoO0kdxONWWNOxddood       \n"
           "d:::::ccoxO0KK0kdlllllcldk0KKK0Oxdddoood       \n"
           "d::;:::::::ccc;,,,,,,,,,,,:loooooolllllo       \n"
           "Oc;;;;;;::::::;,''''''''',:cccccccc::::l       \n"
           "NOc;;;;;;;;;;;;;,,,,,,,,,;;;;;;;;;;;;;lO       \n"
           " WKxl;,,,,,,,,,,,,,,,,,,,,,,,,,,,,,:lkXW       \n"
           "   WXOdc:;;;;;;;;;;;;;;;;;;;;;;;:cdONW         \n");
}

/**
 * Run the shell.
 *
 * @return error code.
 * */
int run() {
    print_banner();

    // DONE: save the current path
    char *prelude_path = strdup(getenv(ENV_PATH));
    assert(prelude_path);

    // DONE: find the user home directory from the environment
    // DONE: set current working directory to the user home directory
    change_directory("~");

    ShiteShellState state = {0};
    // DONE: load history
    create_history(&state.history);
    load_history(&state.history);

    // DONE: load aliases
    load_aliases();

    char input_buf[MAX_COMMAND_SIZE] = {0};

    // NOTE: we set the token_buf to the size of the input buf since it
    // guarantees we'll never run out of space in the token buffer. char
    // *token_buf[MAX_COMMAND_SIZE] = {0};

    while (true) {
        display_prompt();

        char *input = get_input(input_buf, sizeof(input_buf));

        // NOTE: special case for CTRL+D
        if (!input)
            break;

        // Skip empty input
        if (input[0] == '\0')
            continue;

        char *tokens[MAX_COMMAND_SIZE] = {0};
        tokenize_input(input, tokens);
        if (!tokens[0])
            continue;

        // NOTE: special case handle internal command exit
        if (STR_EQ("exit", tokens[0]) && !tokens[1])
            break;

        // DONE: execute command
        execute_command(tokens);
    }

    save_history(&state.history);

    save_aliases();

    printf("\n");
    printf("\n");
    printf("Restoring original path...\n");
    int err = setenv(ENV_PATH, prelude_path, true);
    assert(!err);
    printf("Path restored to:\n");
    printf("\"%s\"\n", prelude_path);

    printf("Exiting normally...\n");

    for (int i = 0; i < HIST_SIZE; i++) {
        if (state.history.commands[i])
            free(state.history.commands[i]);
    }
    free(prelude_path);

    return 0;
}
