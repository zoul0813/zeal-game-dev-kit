#include <core.h>
#include "private.h"

zos_err_t zmt_rom_load(track_t *track, const uint8_t *start, uint16_t len)
{
    if (track == NULL || start == NULL) return ERR_INVALID_PARAMETER;
    tracker_rom_cursor_t cursor = {start, len};
    uint8_t header[4], tempo, count;
    char title[TRACKER_TITLE_LEN];
    if (!tracker_rom_read(&cursor, header, sizeof(header))
        || header[0] != 'Z' || header[1] != 'M' || header[2] != 'T' || header[3] != 0
        || !tracker_rom_read(&cursor, title, sizeof(title))
        || !tracker_rom_read(&cursor, &tempo, 1)
        || !tracker_rom_read(&cursor, &count, 1)
        || count == 0 || count > NUM_PATTERNS) return ERR_ENTRY_CORRUPTED;
    for (uint8_t p = 0; p < count; p++)
        if (track->patterns[p] == NULL) return ERR_INVALID_PARAMETER;

    /* Validate the whole payload before modifying track or playback state. */
    tracker_rom_cursor_t payload;
    payload = cursor;
    if (!tracker_rom_arrangement(&cursor, NULL, count)) return ERR_ENTRY_CORRUPTED;
    for (uint8_t p = 0; p < count; p++)
        if (!tracker_rom_pattern(&cursor, NULL)) return ERR_ENTRY_CORRUPTED;

    zmt_track_init(track);
    mem_cpy(track->title, title, sizeof(title));
    track->title[TRACKER_TITLE_LEN] = 0;
    track->tempo = track->current_tempo = tempo;
    track->pattern_count = count;
    tracker_rom_arrangement(&payload, track->arrangement, count);
    for (uint8_t p = 0; p < count; p++)
        tracker_rom_pattern(&payload, track->patterns[p]);
    return ERR_SUCCESS;
}
