#include <string.h>

#define STR_EQ(a, b) (0 == strcmp(a, b))
#define ARRAY_LEN(arr) ((int)(sizeof(arr) / sizeof(arr[0])))
#define ENV_PATH "PATH"
#define ENV_HOME "HOME"

#define MAX_COMMAND_SIZE 512

void change_directory(const char *new_path);
