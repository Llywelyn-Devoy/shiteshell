# Welcome to ShiteShell

It's a basic implementation of a bash-like shell with the following features:

- command running
- aliasing
  - supports meta-aliases, (aliases of aliases)
  - simple cycle detection by limiting recursion depth
- history
- directory stack, (new feature)
  - `cd -` to navigate to the previous directory

This was a team-based project for the `CS210 Computer Systems and Architecture`
module following [this brief](./simpleShell.pdf), that I've retroactively
refactored and extended with new features.


## Building and running

### LINUX

If you're on Linux, type `make` and there should then be an executable
under `./bin/`.

### WINDOWS

If you're on Windows, probably best to use WSL and follow the Linux
steps above.


## Formatting 

Please use `clang-format` with your chosen editor, we've got a `.clang-format`
config file.
