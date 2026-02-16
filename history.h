#ifndef HISTORY_H
#define HISTORY_H
#define HIST_SIZE 20

struct history_arr {
  char *commands[HIST_SIZE];
  int count;
  int overflow;
};

void create_history(struct history_arr *history);

const char *last_command(struct history_arr *history);

char *load_history_command(struct history_arr *history, char *dest_buf,
                           const char *input_buf);

void print_commands(struct history_arr *h);

void add_command_to_history(struct history_arr *h, const char *command);

void load_history(struct history_arr *history);

void save_history(struct history_arr *history);

#endif