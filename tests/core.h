/* Host substitutes for the two core memory functions used by tiles.c. */
#include <string.h>
#define mem_set memset
#define mem_cpy memcpy
