#include <core.h>
#include "private.h"

uint8_t tracker_rom_pattern(tracker_rom_cursor_t *cursor, pattern_t *out)
{
    uint8_t voices;
    if (!tracker_rom_read(cursor, &voices, 1) || (voices & 0xF0)) return 0;
    if (out != NULL) zmt_pattern_init(out);
    for (uint8_t i = 0; i < NUM_VOICES; i++) {
        if (!(voices & (1U << i))) continue;
        uint8_t bitmap[4];
        if (!tracker_rom_read(cursor, bitmap, sizeof(bitmap))) return 0;
        for (uint8_t j = 0; j < STEPS_PER_PATTERN; j++) {
            if (!(bitmap[j >> 3] & (1U << (j & 7)))) continue;
            uint8_t header;
            step_t step;
            zmt_step_init(&step, j);
            if (!tracker_rom_read(cursor, &header, 1) || (header & 0xF0)) return 0;
            if (header & STEP_CELL_NOTE) {
                if (!tracker_rom_read(cursor, &step.note, 1)) return 0;
                if (step.note >= NUM_NOTES && step.note != NOTE_OUT_OF_RANGE) return 0;
            }
            if (header & STEP_CELL_WAVE) {
                if (!tracker_rom_read(cursor, &step.waveform, 1)) return 0;
                if (step.waveform > 3 && step.waveform != WAVEFORM_OUT_OF_RANGE) return 0;
            }
            if ((header & STEP_CELL_FX1) && !tracker_rom_read(cursor, &step.fx1, 1)) return 0;
            if ((header & STEP_CELL_FX2) && !tracker_rom_read(cursor, &step.fx2, 1)) return 0;
            if (out != NULL) out->voices[i].steps[j] = step;
        }
    }
    return 1;
}

/* Legacy API: caller must provide a complete, trusted pattern buffer. */
zos_err_t zmt_pattern_rom_load(pattern_t *pattern, uint8_t **start)
{
    if (pattern == NULL || start == NULL || *start == NULL) return ERR_INVALID_PARAMETER;
    tracker_rom_cursor_t cursor = {*start, UINT16_MAX};
    if (!tracker_rom_pattern(&cursor, NULL)) return ERR_ENTRY_CORRUPTED;
    cursor.pos = *start;
    cursor.remaining = UINT16_MAX;
    tracker_rom_pattern(&cursor, pattern);
    *start = (uint8_t *)cursor.pos;
    return ERR_SUCCESS;
}
