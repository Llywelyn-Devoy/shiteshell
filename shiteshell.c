#include "shiteshell.h"
#include "io.h"
#include "shared.h"
#include <assert.h>
#include <errno.h> // errno
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> // pid_t datatype
#include <sys/wait.h>  // wait
#include <unistd.h>    // fork, execl

typedef enum {
    CMD_NOT_RECOGNIZED,
    CMD_INVALID_FORMAT,
    CMD_SUCCESS,
} CmdResult;

/**
 * Checks if a colon-separated path list is valid. The list is valid if
 * all of the paths within it are valid directories.
 *
 * @param path, colon-separated list of paths
 * @param path_len, the length of the path str
 * @return `true` if the path list is valid, `false` otherwise.
 * */
bool _is_valid_path(const char *path, int path_len) {
    // duplicate the path as to not overwrite it
    char path_dup[path_len + 1];
    memcpy(path_dup, path, sizeof(path_dup));
    char delim[] = ":";
    char *dir_path = strtok(path_dup, delim);
    while (dir_path) {
        if (!directory_exists(dir_path))
            // TODO: return this invalid path, so that we can be more specific
            // to the user about the issue.
            return false;
        dir_path = strtok(NULL, delim);
    }
    return true;
}

CmdResult cmd_getpath(char *const argv[]) {
    const char *const name = argv[0];
    if (!STR_EQ(name, "getpath")) {
        return CMD_NOT_RECOGNIZED;
    }
    if (argv[1]) {
        fprintf(stderr, "ERROR: `%s` expects no arguments.\n", name);
        return CMD_INVALID_FORMAT;
    }

    const char *path = getenv(ENV_PATH);
    assert(path);
    printf("%s\n", path);
    return CMD_SUCCESS;
}

CmdResult cmd_setpath(char *const argv[]) {
    const char *const name = argv[0];
    if (!STR_EQ(name, "setpath")) {
        return CMD_NOT_RECOGNIZED;
    }

    const char *new_path = argv[1];
    const char *null = argv[2];
    if (!new_path || null != NULL) {
        fprintf(
            stderr,
            "ERROR: Invalid arguments. "
            "`%s` expects one argument; a colon-separated list to set the path "
            "to.\n",
            name);
        return CMD_INVALID_FORMAT;
    }

    if (!_is_valid_path(new_path, strlen(new_path))) {
        fprintf(stderr,
                "ERROR: path provided is invalid. It must be "
                "colon-separated and all paths within must be valid.\n");
        return CMD_INVALID_FORMAT;
    }

    int err = setenv(ENV_PATH, new_path, true);
    assert(!err);

    printf("\"%s\" has been set.\n", ENV_PATH);
    return CMD_SUCCESS;
}

CmdResult cmd_change_dir(char *const argv[]) {
    const char *name = argv[0];
    const char *path = argv[1];
    const char *null = argv[2];
    if (!STR_EQ(name, "cd")) {
        return CMD_NOT_RECOGNIZED;
    }
    if (null != NULL) {
        fprintf(
            stderr,
            "ERROR: invalid arguments."
            "`%s` requires one optional argument for the path to change to.\n",
            name);
        return CMD_INVALID_FORMAT;
    }

    if (path) {
        change_directory(path);
    } else {
        change_directory("~"); // default to user home
    }
    return CMD_SUCCESS;
}

CmdResult cmd_alias(ShiteShellState *state, char *const argv[]) {
    const char *name = argv[0];
    if (!STR_EQ(name, "alias")) {
        return CMD_NOT_RECOGNIZED;
    }

    const char *alias_name = argv[1];
    if (!alias_name) {
        aliasstore_print(&state->alias_store);
        return CMD_SUCCESS;
    }

    if (STR_EQ(alias_name, "alias")) {
        fprintf(stderr, "ERROR: cannot create alias with name \"alias\", "
                        "restricted name.\n");
        return CMD_INVALID_FORMAT;
    }

    if (STR_EQ(alias_name, "unalias")) {
        fprintf(stderr, "ERROR: cannot create alias with name \"unalias\", "
                        "restricted name.\n");
        return CMD_INVALID_FORMAT;
    }

    char alias_cmd[MAX_COMMAND_SIZE] = {0};

    if (!argv[2]) {
        fprintf(stderr, "ERROR: alias expects a command to be aliased.\n");
        return CMD_INVALID_FORMAT;
    }

    for (int i = 2; argv[i] != NULL; i++) {
        if (i > 2) {
            strcat(alias_cmd, " ");
        }
        strcat(alias_cmd, argv[i]);
    }

    aliasstore_add_alias(&state->alias_store, strdup(alias_name),
                         strdup(alias_cmd));
    return CMD_SUCCESS;
}

CmdResult cmd_unalias(ShiteShellState *state, char *const argv[]) {
    const char *name = argv[0];
    if (!STR_EQ(name, "unalias")) {
        return CMD_NOT_RECOGNIZED;
    }

    const char *alias_name = argv[1];
    if (!alias_name) {
        fprintf(stderr, "ERROR: unalias expects an alias name.\n");
        return CMD_INVALID_FORMAT;
    }

    Alias *alias = aliasstore_get_by_name(&state->alias_store, alias_name);
    if (!alias) {
        fprintf(stderr, "ERROR: no such alias with name \"%s\".\n", alias_name);
        return CMD_INVALID_FORMAT;
    }

    aliasstore_rm(&state->alias_store, &alias);
    return CMD_SUCCESS;
}

CmdResult cmd_history_print(ShiteShellState *state, char *const argv[]) {
    const char *name = argv[0];
    const char *null = argv[1];
    if (!STR_EQ(name, "history")) {
        return CMD_NOT_RECOGNIZED;
    }

    if (null != NULL) {
        return CMD_INVALID_FORMAT;
    }

    history_print(&state->history);
    return CMD_SUCCESS;
}

CmdResult cmd_history_invoke(ShiteShellState *state, char *const argv[],
                             char *new_input_out) {
    const char *name = argv[0];
    const char *null = argv[1];
    if (!STR_STARTSWITH(name, "!")) {
        return CMD_NOT_RECOGNIZED;
    }
    if (null != NULL) {
        fprintf(stderr, "ERROR: too many arguments.\n");
        return CMD_INVALID_FORMAT;
    }

    const int count = history_count(&state->history);
    if (count == 0) {
        printf("INFO: History is empty\n");
        return CMD_INVALID_FORMAT;
    }

    long offset = 0;
    bool err_parse = false;
    if (STR_EQ(name, "!!")) {
        offset = -1;
    } else {
        const char *number_part = name + strlen("!");
        // We are assuming tokenisation of spaces
        assert(!STR_STARTSWITH(number_part, " "));

        // TODO: break out number parsing
        if (STR_STARTSWITH(number_part, "+")) { // we don't accept numbers
            // starting with '+'
            err_parse = true;
        } else {
            errno = 0; // reset errno so we can read it
            char *end = NULL;
            offset = strtol(number_part, &end, 10);
            err_parse = errno == ERANGE || !STR_EQ(end, "");
        }
    }

    if (err_parse) {
        fprintf(stderr, "ERROR: invalid number format.\n");
        return CMD_INVALID_FORMAT;
    }

    const int idx = (offset > 0) ? offset - 1 : count + offset;
    const char *history_item = history_get(&state->history, idx);
    if (offset == 0 || !history_item) {
        fprintf(stderr, "ERROR: no such history item.\n");
        return CMD_INVALID_FORMAT;
    }

    strcpy(new_input_out, history_item);
    return CMD_SUCCESS;
}

#define TOK_MATCH(tokens, ...) tok_match(tokens, (char *[]){__VA_ARGS__, NULL})

void display_prompt() {
    char cwd[PATH_MAX];
    if (!getcwd(cwd, sizeof(cwd))) {
        // TODO: maybe we don't want to crash here?
        perror("getcwd() error");
        exit(-1);
    }

    const char *format_str = "%s $> ";
    // try to remove home path
    const char *home_path = (const char *)getenv("HOME");
    if (STR_STARTSWITH(cwd, home_path)) {
        char pretty_path[PATH_MAX];
        const char *slice = cwd + strlen(home_path);
        strcpy(pretty_path, "~");
        strcat(pretty_path, slice);
        printf(format_str, pretty_path);
    } else {
        printf(format_str, cwd);
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

bool tok_match(char **tokens, char **pattern) {
    for (int i = 0;; i++) {
        const char *tok = tokens[i];
        const char *pat = pattern[i];
        if (tok == NULL && pat == NULL)
            break;
        else if (tok == NULL || pat == NULL)
            return false;
        else if (!STR_EQ(tok, pat))
            return false;
    }
    return true;
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

    exec(tokens[0], tokens);
}

void print_banner() {
    printf("+-------------------------------------------------+\n"
           "|   _____ _     _ _        _____ _          _ _   |\n"
           "|  / ____| |   (_) |      / ____| |        | | |  |\n"
           "| | (___ | |__  _| |_ ___| (___ | |__   ___| | |  |\n"
           "|  \\___ \\| '_ \\| | __/ _ \\\\___ \\| '_ \\ / _ \\ | |  |\n"
           "|  ____) | | | | | ||  __/____) | | | |  __/ | |  |\n"
           "| |_____/|_| |_|_|\\__\\___|_____/|_| |_|\\___|_|_|  |\n"
           "|                                                 |\n"
           "|                          W0oco0W                |\n"
           "|                         N0occcdK                |\n"
           "|                     WN0kdlcllllkN               |\n"
           "|                  WX0kolcclooolcdX               |\n"
           "|             WNKOxdlllooooodol::lk0NW            |\n"
           "|           WKxl:::cllloolllc:;;:clodOXW          |\n"
           "|          Xkc;;;:clcc:::::::cclooooooxKW         |\n"
           "|         Xd:;;;;;::ccclllloooooooooooldK         |\n"
           "|        W0c:coxkOOkdlccllllldk0000Oxlcl0W        |\n"
           "|         0oxKW NK00KKklcccoOKK00XWWN0do0W        |\n"
           "|      WXOdON WOc,'';dKkc:l0Oc,'';dX  Xxx0N       |\n"
           "|     NOocl0  Xc.....'k0l;d0l.....,xW WOooxKW     |\n"
           "|    Xxc::ckW Nx,....c0Oc;l0x;...'c0  Nkoood0     |\n"
           "|    kc::::lkXWWKkdxOKOdlccoO0kdxONWWNOxddood     |\n"
           "|    d:::::ccoxO0KK0kdlllllcldk0KKK0Oxdddoood     |\n"
           "|    d::;:::::::ccc;,,,,,,,,,,,:loooooolllllo     |\n"
           "|    Oc;;;;;;::::::;,''''''''',:cccccccc::::l     |\n"
           "|    NOc;;;;;;;;;;;;;,,,,,,,,,;;;;;;;;;;;;;lO     |\n"
           "|     WKxl;,,,,,,,,,,,,,,,,,,,,,,,,,,,,,:lkXW     |\n"
           "|       WXOdc:;;;;;;;;;;;;;;;;;;;;;;;:cdONW       |\n"
           "+-------------------------------------------------+\n");
}

/**
 * Run the shell.
 *
 * @return error code.
 * */
int run(ShiteShellOpts *opts) {
    if (opts->no_banner == false)
        print_banner();

    // DONE: save the current path
    char *prelude_path = strdup(getenv(ENV_PATH));
    assert(prelude_path);
    DEFER(shite_free((void **)&prelude_path)) {

        // DONE: find the user home directory from the environment
        // DONE: set current working directory to the user home directory
        change_directory("~");

        ShiteShellState state = {0};
        // DONE: load history
        history_load(&state.history);

        // DONE: load aliases
        aliasstore_load(&state.alias_store);

        while (true) {
            display_prompt();

            char input_buf[MAX_COMMAND_SIZE] = {0};
            const char *input = get_input(input_buf, sizeof(input_buf));

            // NOTE: special case for CTRL+D
            if (!input)
                break;

            // Skip empty input
            if (STR_EQ(input, ""))
                continue;

            char mut_input[MAX_COMMAND_SIZE] = {0};
            strcpy(mut_input, input);
            char *tokens[MAX_COMMAND_SIZE] = {0};

            const int ITERATION_LIMIT = 64;
            bool push_to_history = true;
            bool re_prompt = false;
            for (int iteration = 0;; iteration++) {
                if (re_prompt)
                    break;

                if (iteration >= ITERATION_LIMIT) {
                    fprintf(stderr,
                            "ERROR: alias resolution has reached "
                            "iteration limit, (%d). Maybe there is a cycle "
                            "within your aliases?\n",
                            ITERATION_LIMIT);
                    re_prompt = true;
                    continue;
                }

                tokenize_input(mut_input, tokens);
                if (!tokens[0]) {
                    re_prompt = true;
                    continue;
                }

                // NOTE: in order for alias and unalias commands to not act
                // weird we need to special case these commands _before_ alias
                // resolution.
                if (CMD_NOT_RECOGNIZED != cmd_alias(&state, tokens)) {
                    re_prompt = true;
                    continue;
                }

                if (CMD_NOT_RECOGNIZED != cmd_unalias(&state, tokens)) {
                    re_prompt = true;
                    continue;
                }

                char new_input[MAX_COMMAND_SIZE] = {0};
                switch (aliasstore_resolve_alias(
                    &state.alias_store, tokens, new_input, sizeof(new_input))) {
                case ARR_NO_CHANGE:
                    break;
                case ARR_SUCCESS:
                    strcpy(mut_input, new_input);
                    continue;
                    break;
                case ARR_FAILURE:
                    fprintf(
                        stderr,
                        "ERROR: alias resolution expanded beyond the maximum "
                        "supported command size.\n");
                    re_prompt = true;
                    continue;
                }

                if (TOK_MATCH(tokens, "exit")) {
                    goto exit;
                }

                if (CMD_NOT_RECOGNIZED != cmd_history_print(&state, tokens)) {
                    push_to_history = false;
                    re_prompt = true;
                    continue;
                }

                switch (cmd_history_invoke(&state, tokens, mut_input)) {
                case CMD_INVALID_FORMAT:
                    re_prompt = true;
                    continue;
                case CMD_SUCCESS:
                    push_to_history = false;
                    continue;
                default:
                    break;
                }

                break;
            }

            if (push_to_history) {
                // only push to history if the new command doesn't match the
                // last history item
                const char *history_last = history_peek_back(&state.history);
                if (history_last) {
                    char history_last_buf[MAX_COMMAND_SIZE] = {0};
                    strcpy(history_last_buf, history_last);
                    char *history_last_tok[MAX_COMMAND_SIZE] = {0};
                    tokenize_input(history_last_buf, history_last_tok);

                    char og_input[MAX_COMMAND_SIZE] = {0};
                    strcpy(og_input, input);
                    char *og_input_tok[MAX_COMMAND_SIZE] = {0};
                    tokenize_input(og_input, og_input_tok);

                    if (!tok_match(og_input_tok, history_last_tok))
                        history_push_back(&state.history, strdup(input));
                } else {
                    history_push_back(&state.history, strdup(input));
                }
            }

            if (re_prompt)
                continue;

            if (CMD_SUCCESS == cmd_getpath(tokens))
                continue;
            if (CMD_SUCCESS == cmd_setpath(tokens))
                continue;
            if (CMD_SUCCESS == cmd_change_dir(tokens))
                continue;

            // DONE: execute command
            execute_command(tokens);
        }
    exit:

        history_save(&state.history);
        history_free(&state.history);

        aliasstore_save(&state.alias_store);
        aliasstore_free(&state.alias_store);

        printf("\n");
        printf("\n");
        printf("Restoring original path...\n");
        int err = setenv(ENV_PATH, prelude_path, true);
        assert(!err);
        printf("Path restored to:\n");
        printf("\"%s\"\n", prelude_path);

        printf("Exiting normally...\n");
    }
    return 0;
}
