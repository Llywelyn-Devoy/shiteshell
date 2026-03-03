#pragma once

#include "shared.h"
#include <stdio.h>
#include <stdbool.h>

// FIXME: this is nasty 
#include "shiteshell.h"

#define MAX_ALIASES 10

#define ALIAS_COMMAND_NAME "alias"
#define UNALIAS_COMMAND_NAME "unalias"

typedef struct {
  char name[64];
  char command[MAX_COMMAND_SIZE];
} Alias;

/**
 * Returns the index of the alias, otherwise -1.
 *
 * @param name name of the alias
 * @return -1 if the alias is not found, otherwise the index of the alias.
 * */
int index_of_alias(const char *name);

/**
 * Takes a null-terminated array of tokens and attempts to create a new alias.
 * Errors are written to stderr
 *
 * @param tokens[], null-terminated array of token strings.
 * */
void alias_command(char *const tokens[]);

/**
 * Takes a null-terminated array of tokens and attempts to remove a defined
 * alias. Errors are written to stderr
 *
 * @param tokens[], null-terminated array of token strings.
 * */
void unalias_command(char *const tokens[]);


/**
 * Takes a raw input string, and attempts resolve all tokens that are aliases.
 * This operation is done, exactly once for each alias found.
 *
 * @param input, the raw string from the user
 * @param output, a buffer to write the resolved input string into. Buffer must
 * be atleast MAX_COMMAND_SIZE in size. Behaviour is undefined is the buffer
 * isn't big enough.
 * */
void resolve_alias(const char *input, char *output);