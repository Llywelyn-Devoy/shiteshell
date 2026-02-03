
struct history_arr;

struct history_arr create_history();

void add_command_to_history(char** command, struct history_arr* history_arr);

void print_commands(struct history_arr* history_arr);