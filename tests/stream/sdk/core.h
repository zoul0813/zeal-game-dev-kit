/* Host stub for coreutils' core.h: its mem_cpy copies forward (LDIR), which
 * stream_buffer.c relies on for overlapping copies toward lower addresses. */
#include <string.h>
#define mem_cpy memmove
