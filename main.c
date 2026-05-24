#include "io.h"
#include "shared.h"
#include "shiteshell.h"

typedef struct {
    char *name_short;
    char *name_long;
    void (*applicator)(ShiteShellOpts *);
} Option;

void apply_no_banner(ShiteShellOpts *opts) { opts->no_banner = true; }

static const Option OPTIONS[] = {{.name_short = "-nb",
                                  .name_long = "--no-banner",
                                  .applicator = apply_no_banner}};

ShiteShellOpts parse_options(int argc, char **argv) {
    ShiteShellOpts opts = {0};

    // NOTE: start at 1 to skip program name.
    for (int i = 1; i < argc; i++) {
        bool match = false;
        const char *arg = argv[i];
        for (int j = 0; j < ARRAY_LEN(OPTIONS); j++) {
            if (STR_EQ(arg, OPTIONS[j].name_short) ||
                STR_EQ(arg, OPTIONS[j].name_long)) {
                OPTIONS[j].applicator(&opts);
                match = true;
            }
        }
        if (!match) {
            printf("Unrecognised option: \"%s\".\n", arg);
            exit(1);
        }
    }

    return opts;
}

int main(int argc, char **argv) {
    ShiteShellOpts opts = parse_options(argc, argv);
    return run(&opts);
}
