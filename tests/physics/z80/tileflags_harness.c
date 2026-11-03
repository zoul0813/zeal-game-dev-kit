/* Run the behavior regression against the production Z80 assembly. */
#include <stdio.h>
#include <assert.h>
#undef assert
#define assert(condition) do { if (!(condition)) return __LINE__; } while (0)

/* Console output is unused in the emulator; main's result reports failure. */
int putchar(int c) { return c; }

#include "../tileflags.c"
