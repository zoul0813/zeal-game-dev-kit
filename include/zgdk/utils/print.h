#include <stdint.h>
#include <zvb_gfx.h>

#ifndef UTILS_PRINT_H
#define UTILS_PRINT_H

/**
 * Print a string to layer1 using the tiles defined by ascii_map()
 *
 * @param ctx   gfx_context to use
 * @param str   the null-terminated string to print
 * @param x     the tile x coordinate
 * @param y     the tile y coordinate
 */
inline void print_string(gfx_context* ctx, const char* str, uint8_t x, uint8_t y);
inline void print_string_layer(gfx_context* ctx, const char* str, uint8_t layer, uint8_t x, uint8_t y);

/**
 * Print a string to layer1 using the tiles defined by ascii_map()
 *
 * @param ctx   gfx_context to use
 * @param str   the string to print
 * @param len   the length of the string
 * @param x     the tile x coordinate
 * @param y     the tile y coordinate
 */
inline void nprint_string(gfx_context* ctx, const char* str, uint8_t len, uint8_t x, uint8_t y);
void nprint_string_layer(gfx_context* ctx, const char* str, uint8_t len, uint8_t layer, uint8_t x, uint8_t y);

#endif