#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

#define DEBUG true

#define dprintf(...) do { if (DEBUG) fprintf(stderr, __VA_ARGS__); } while (0)

#endif // DEBUG_H
