#include "io.h"
#include "shared.h"
#include "shiteshell.h"

typedef struct {
    char *name_short;
    char *name_long;
    char *description;
    void (*applicator)(ShiteShellOpts *);
} Option;

void apply_no_banner(ShiteShellOpts *opts) { opts->no_banner = true; }

void show_usage();
void apply_help(ShiteShellOpts *_) {
    show_usage();
    exit(0);
}

static const Option OPTIONS[] = {
    {.name_short = "-h",
     .name_long = "--help",
     .description = "Show the help and usage.\n",
     .applicator = apply_help},
    {.name_short = "-nb",
     .name_long = "--no-banner",
     .description = "Don't show the shell banner.\n",
     .applicator = apply_no_banner}};

void show_usage() {
    printf("Usage: shiteshell OPTIONS\n");
    for (int i = 0; i < ARRAY_LEN(OPTIONS); i++) {
        // TODO: nice padding
        const Option opt = OPTIONS[i];
        printf("%s, %s: %s", opt.name_short, opt.name_long, opt.description);
    }
}

ShiteShellOpts parse_options(int argc, char **argv) {
    ShiteShellOpts opts = {0};

    for (int pass = 0; pass < 2; pass++) {
        // NOTE: start at 1 to skip program name.
        for (int i = 1; i < argc; i++) {
            bool match = false;
            const char *arg = argv[i];
            for (int o = 0; o < ARRAY_LEN(OPTIONS); o++) {
                const Option opt = OPTIONS[o];
                if (STR_EQ(arg, opt.name_short) || STR_EQ(arg, opt.name_long)) {
                    match = true;
                    if (pass == 1) { // only apply arguments on the second pass
                        opt.applicator(&opts);
                    }
                }
            }
            if (!match) {
                printf("Unrecognised option: \"%s\".\n", arg);
                show_usage();
                exit(1);
            }
        }
    }

    return opts;
}

int main(int argc, char **argv) {
    ShiteShellOpts opts = parse_options(argc, argv);
    return run(&opts);
}
