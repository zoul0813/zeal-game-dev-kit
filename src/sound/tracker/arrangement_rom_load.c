#include <core.h>
#include "private.h"

uint8_t tracker_rom_arrangement(tracker_rom_cursor_t *cursor, arrangement_t *out, uint8_t count)
{
    uint8_t bitmap[8];
    if (!tracker_rom_read(cursor, bitmap, sizeof(bitmap))) return 0;
    for (uint8_t i = 0; i < NUM_ARRANGEMENTS; i++) {
        arrangement_t cell = {ARRANGEMENT_OUT_OF_RANGE, FX_OUT_OF_RANGE};
        if (bitmap[i >> 3] & (1U << (i & 7))) {
            if (!tracker_rom_read(cursor, &cell, sizeof(cell))) return 0;
            if (cell.pattern_index != ARRANGEMENT_OUT_OF_RANGE && cell.pattern_index >= count)
                return 0;
        }
        if (out != NULL) out[i] = cell;
    }
    return 1;
}

/* Legacy API: caller must provide a complete, trusted arrangement buffer. */
zos_err_t zmt_arrangement_rom_load(arrangement_t arrangement[NUM_ARRANGEMENTS], uint8_t **start)
{
    if (arrangement == NULL || start == NULL || *start == NULL) return ERR_INVALID_PARAMETER;
    tracker_rom_cursor_t cursor = {*start, UINT16_MAX};
    if (!tracker_rom_arrangement(&cursor, NULL, NUM_PATTERNS)) return ERR_ENTRY_CORRUPTED;
    cursor.pos = *start;
    cursor.remaining = UINT16_MAX;
    tracker_rom_arrangement(&cursor, arrangement, NUM_PATTERNS);
    *start = (uint8_t *)cursor.pos;
    return ERR_SUCCESS;
}
