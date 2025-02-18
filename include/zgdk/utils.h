#pragma once

#include <stdint.h>
#include <zvb_gfx.h>

/**
 * TSTATE_LOG will write `counter` to the "debug_register" at 0x86
 *
 * @param counter The value to log
 */
#ifdef EMULATOR
__sfr __at(0x86) debug_register; // t-state counter
#define TSTATE_LOG(counter) debug_register = counter;
#else
#define TSTATE_LOG(counter)
#endif

/**
 * Print a string to layer1 using the tiles defined by ascii_map()
 *
 * @param ctx   gfx_context to use
 * @param str   the null-terminated string to print
 * @param x     the tile x coordinate
 * @param y     the tile y coordinate
 */
void print_string(gfx_context* ctx, const char* str, uint8_t x, uint8_t y);

/**
 * Print a string to layer1 using the tiles defined by ascii_map()
 *
 * @param ctx   gfx_context to use
 * @param str   the string to print
 * @param len   the length of the string
 * @param x     the tile x coordinate
 * @param y     the tile y coordinate
 */
void nprint_string(gfx_context* ctx, const char* str, uint8_t len, uint8_t x, uint8_t y);

/**
 * Generate a quick 8-bit random number using the Z80's R register
 *
 * @return Value of the R register, 8-bit pseudo random value
 */
char rand8_quick(void);

/**
 * Generate an 8-bit random number using a combination of an LFSR w/ Counter
 *
 * @return 8-bit random value
 */
char rand8(void);