#include "io.h"
#include "shared.h"
#include "shiteshell.h"

int main(int argc, char **argv) {
    ShiteShellOpts opts = {.no_banner = argc == 2 && STR_EQ(argv[1], "-nb")};
    return run(&opts);
}
