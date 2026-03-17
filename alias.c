#include "shared.h"
#include <stdbool.h>
#include <stdio.h>


// FIXME: this is nasty, we don't want this weird two-way dependency
#include "alias.h"
#include "shiteshell.h"

Alias aliases[MAX_ALIASES];
int no_alias = 0;

/**
 * Returns the index of the alias, otherwise -1.
 *
 * @param name name of the alias
 * @return -1 if the alias is not found, otherwise the index of the alias.
 * */
int index_of_alias(const char *name) {
    for (int i = 0; i < no_alias; i++) {
        if (strcmp(aliases[i].name, name) == 0)
            return i;
    }
    return -1;
}

/**
 * Takes a null-terminated array of tokens and attempts to create a new alias.
 * Errors are written to stderr
 *
 * @param tokens[], null-terminated array of token strings.
 * */
void alias_command(char *const tokens[]) {
    const char *alias_name = tokens[1];
    char *const *rest = &tokens[2];

    if (!alias_name) {
        for (int i = 0; i < no_alias; i++) {
            printf("%s = '%s'\n", aliases[i].name, aliases[i].command);
        }
        return;
    }

    if (STR_EQ(ALIAS_COMMAND_NAME, alias_name)) {
        fprintf(stderr, "ERROR: cannot create alias with name \"%s\".\n",
                ALIAS_COMMAND_NAME);
        return;
    }
    if (STR_EQ(UNALIAS_COMMAND_NAME, alias_name)) {
        fprintf(stderr, "ERROR: cannot create alias with name \"%s\".\n",
                UNALIAS_COMMAND_NAME);
        return;
    }

    if (!rest[0]) {
        fprintf(stderr, "ERROR: must supply a command to alias.\n");
        return;
    }

    char command[MAX_COMMAND_SIZE] = {0};
    for (int i = 2; tokens[i]; i++) {
        strcat(command, tokens[i]);
        if (tokens[i + 1])
            strcat(command, " ");
    }

    int alias_idx = index_of_alias(alias_name);

    if (alias_idx >= 0) {
        printf("INFO: redefined alias \"%s\".\n", alias_name);
        strcpy(aliases[alias_idx].command, command);
    } else {
        if (no_alias >= MAX_ALIASES) {
            fprintf(stderr,
                    "ERROR: wooh there bucko stop renaming stuff 10 max.\n");
            return;
        }
        strcpy(aliases[no_alias].name, alias_name);
        strcpy(aliases[no_alias].command, command);
        no_alias++;
    }
}

/**
 * Takes a null-terminated array of tokens and attempts to remove a defined
 * alias. Errors are written to stderr
 *
 * @param tokens[], null-terminated array of token strings.
 * */
void unalias_command(char *const tokens[]) {
    if (!tokens[1]) {
        fprintf(stderr, "ERROR: must supply an alias to unalias.\n");
        return;
    }

    int alias_idx = index_of_alias(tokens[1]);
    if (alias_idx < 0) {
        fprintf(stderr, "ERROR: no such alias.\n");
        return;
    }

    for (int i = alias_idx; i < no_alias - 1; i++) {
        aliases[i] = aliases[i + 1];
    }
    no_alias--;
}

/**
 * Takes a raw input string, and attempts resolve all tokens that are aliases.
 * This operation is done, exactly once for each alias found.
 *
 * @param input, the raw string from the user
 * @param output, a buffer to write the resolved input string into. Buffer must
 * be atleast MAX_COMMAND_SIZE in size. Behaviour is undefined is the buffer
 * isn't big enough.
 * */
void resolve_alias(const char *input, char *output, bool expanded_alias[MAX_COMMAND_SIZE][MAX_ALIASES]) {

    char current[MAX_COMMAND_SIZE];
    strcpy(current, input);

    while (1) {

        char work[MAX_COMMAND_SIZE];
        strcpy(work, current);

        char *tokens[MAX_COMMAND_SIZE] = {0};
        tokenize_input(work, tokens);

        if (!tokens[0])
            break;

        if (strcmp(tokens[0], UNALIAS_COMMAND_NAME) == 0)
            break;

        if (strcmp(tokens[0], ALIAS_COMMAND_NAME) == 0)
            break;

        char next[MAX_COMMAND_SIZE] = {0};
        bool changed = false;

        for (int i = 0; tokens[i]; i++) {

            int id = index_of_alias(tokens[i]);

            if (id >= 0 && !expanded_alias[i][id]) {

                strncat(next, aliases[id].command, MAX_COMMAND_SIZE - strlen(next) - 1);

                expanded_alias[i][id] = true;
                changed = true;

            } else {
                if (id >= 0 && expanded_alias[i][id]){
                    printf("INFO: Looping behaviour halted resolution of alias: %s\n", tokens[i]);
                }

                strncat(next, tokens[i], MAX_COMMAND_SIZE - strlen(next) - 1);
            }

            if (tokens[i + 1])
                strncat(next, " ", MAX_COMMAND_SIZE - strlen(next) - 1);
        }

        if (!changed)
            break;

        strcpy(current, next);
    }

    strcpy(output, current);
}

/**
 * Saves the current contents of the aliases data structure into the .aliases file
 * in the user's home directory.
 */
void save_aliases() {
    const char *home = getenv("HOME");
    if (!home) return;

    char path[MAX_COMMAND_SIZE];
    snprintf(path, sizeof(path), "%s/.aliases", home);

    FILE *file = fopen(path, "w");
    if (!file) {
        // If file fails to open, we just return as per "abnormal conditions" check
        return;
    }

    for (int i = 0; i < no_alias; i++) {
        // Format: alias-name command
        fprintf(file, "%s %s\n", aliases[i].name, aliases[i].command);
    }

    fclose(file);
}


void load_aliases() {
    const char *home = getenv("HOME");
    if (!home) return;

    char path[MAX_COMMAND_SIZE];
    snprintf(path, sizeof(path), "%s/.aliases", home);

    FILE *file = fopen(path, "r");
    if (!file) return; // Abnormal condition: file doesn't exist yet

    char line[512]; // Max 512 as per instructions
    while (fgets(line, sizeof(line), file) && no_alias < MAX_ALIASES) {
        line[strcspn(line, "\n")] = 0; // Remove newline

        // First token is alias name, rest is command
        char *space = strchr(line, ' ');
        if (space) {
            *space = '\0';
            char *name = line;
            char *cmd = space + 1;

            // Trim leading spaces for the command
            while (*cmd == ' ') cmd++;

            if (strlen(name) > 0 && strlen(cmd) > 0) {
                strcpy(aliases[no_alias].name, name);
                strcpy(aliases[no_alias].command, cmd);
                no_alias++;
            }
        }
    }
    fclose(file);
}