#include <stdbool.h>

/**
 * Tries to run the specified internal command, returning whether or not it was
 * successful.
 *
 * @param name, the name of the internal command to try and run.
 * @param argv, the arguments, _including_the_internal_commmand name_ to pass to
 * the internal command.
 * @return true if successfully ran the internal command, otherwise false.
 * */
bool try_run_internal_command(char *const name, char *const argv[]);
