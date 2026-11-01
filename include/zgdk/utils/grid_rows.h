#ifndef ZGDK_UTILS_GRID_ROWS_H
#define ZGDK_UTILS_GRID_ROWS_H

#include <stdint.h>
#include "zgdk/inline.h"

/* Build a row-address table for a byte grid, independent of tile properties,
 * rendering or collision. The caller owns both the table and the backing data.
 * rows must have count entries; data must contain count rows of stride bytes.
 * A zero count leaves the table untouched, and permits NULL rows/data.
 * Rebuild if data or stride changes. No bounds checks are performed on access:
 * rows[y][x] reads a byte, whether it is a tile ID, glyph or other grid value.
 * On Z80 each row pointer occupies two bytes, like a uint16_t row offset.
 * Inline so constant dimensions specialize without a per-access wrapper. */
INLINE void grid_rows_init(const uint8_t **rows, const void *data,
                           uint16_t stride, uint8_t count) {
    const uint8_t *row = data;
    while (count--) {
        *rows++ = row;
        row += stride;
    }
}

#endif
