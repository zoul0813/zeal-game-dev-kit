/* Run the behavior regression against the production Z80 assembly. */
#include <stdio.h>

/* Console output is unused in the emulator; main's result reports failure. */
int putchar(int c) { return c; }

#include "../platformer.c"
