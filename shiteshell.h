#include "alias.h"
#include "history.h"
#include <stdbool.h>

typedef struct {
    bool no_banner : 1;
} ShiteShellOpts;

typedef struct {
    History history;
    AliasStore alias_store;
} ShiteShellState;

int run(ShiteShellOpts *opts);
