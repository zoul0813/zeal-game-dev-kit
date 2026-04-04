#include <core.h>
#include "zgdk/sound/tracker.h"
#include "zgdk/utils/log.h"

zos_err_t zmt_rom_load(track_t* track, const uint8_t* start, uint16_t len) {
    len; /// unreferenced
    zos_err_t err;
    uint16_t size = 0;
    const uint8_t *pos = start;
    char textbuff[TRACKER_TITLE_LEN + 1];

    // TODO: do this after we've determine the pattern count?
    zmt_track_init(track); // clear out the track, before loading data

    size = 3;
    mem_cpy(textbuff, pos, size);
    pos += size;
    // printf("Format: %.3s, pointer: %04x, %u\n", textbuff, pos, size);

    size = sizeof(uint8_t);
    mem_cpy(textbuff, pos, size);
    pos += size;
    // printf("Version: %d, pointer: %04x, %u\n", textbuff[0], pos, size);

    size = TRACKER_TITLE_LEN;
    mem_cpy(track->title, textbuff, size);
    pos += size;
    track->title[TRACKER_TITLE_LEN] = 0x00; // NUL
    // printf("Track: %12s (read: %d), pointer: %04x, %u\n", track->title, size, pos, size);

    size = sizeof(uint8_t);
    mem_cpy(&track->tempo, pos, size);
    pos += size;
    // printf("Tempo: %d, pointer: %04x, %u\n", track->tempo, pos, size);

    track->current_tempo = track->tempo;

    size = sizeof(uint8_t);
    mem_cpy(&track->pattern_count, pos, size);
    pos += size;
    // printf("Patterns: %d, pointer: %04x, %u\n", track->pattern_count, pos, size);

    err = zmt_arrangement_rom_load(track->arrangement, &pos);
    if (err != ERR_SUCCESS) {
        log_error("error reading arrangement", err);
        return err;
    }
    // printf("Arrangements: %04x\n", pos);
    // for(uint8_t i = 0; i < NUM_ARRANGEMENTS; i++) {
    //     printf("%02x ", track->arrangement[i].pattern_index);
    // }
    // printf("\n");

    for (uint8_t p = 0; p < track->pattern_count; p++) {
        // printf("Loading pattern %d, %04x\n", p, pos);
        err = zmt_pattern_rom_load(track->patterns[p], &pos);
        if (err != ERR_SUCCESS) {
            log_error("error loading patterns", err);
            return err;
        }
    }

    // printf("File loaded.\n\n");
    // err = close(file_dev);
    // return ERR_NOT_IMPLEMENTED; // err;
    return ERR_SUCCESS;
}
