#include <stdbool.h>

typedef struct {
    bool no_banner : 1;
} ShiteShellOpts;

int run(ShiteShellOpts *opts);
