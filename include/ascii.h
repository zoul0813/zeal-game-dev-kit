#include <stdio.h>
#include <stdint.h>

#ifndef ASCII_H
#define ASCII_H

#define ASCII_MAP_OFFSET    0x20U

/**
 * Map of ascii characters, used by various string
 * routines to render text.
 *
 * This map is offset by 0x20 (32) and skips the
 * non-printable characters.
 *
 * This is meant for single line string prints
 */
extern uint8_t ASCII_MAP[95];

/**
 * Map ASCII characters to tile coordinates
 *
 * This allows you to map ASCII 65 to Tile 160
 * for example.  `ascii_map('A', 26, 160) would
 * result in mapping 26 characters from A-Z to
 * tile indexes 160-186
 *
 * @param start the starting ascii value (ie; 'a')
 * @param length the number of characters to map
 * @param from the tile position to begin mapping to
 */
void ascii_map(uint8_t start, uint8_t length, uint8_t from);
void ascii_print(void);

static uint8_t ascii_to(uint8_t c) {
  return ASCII_MAP[c - ASCII_MAP_OFFSET];
}

#endif