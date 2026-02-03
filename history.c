
#define HIST_SIZE 20

struct history_arr{
    char** commands[HIST_SIZE];
    int current_size;
};

struct history_arr create_history(){
    struct history_arr history;
    history.current_size = 0;
    return history;
}

void add_command_to_history(char** command, struct history_arr* history_arr){
    history_arr->commands[history_arr->current_size % HIST_SIZE] = command;
    history_arr->current_size++;
}

void print_commands(struct history_arr* history_arr){
    for (int i = 0; i < HIST_SIZE; i++)
    {
        int world_counter = 0;
        while(history_arr->commands[world_counter++]){
            int letter_counter = 0;
            while(*history_arr->commands[letter_counter++]){
                printf(history_arr[letter_counter]);
            }
            printf(" ");
        }
        printf("/n %d", i);
    }
}