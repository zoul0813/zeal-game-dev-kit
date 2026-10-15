#include <core.h>
#include "zgdk/sound/tracker.h"
#include "private.h"

zos_err_t zmt_file_load(track_t* track, const char* filename)
{
    zos_err_t err;
    uint8_t header[4];
    if (track == NULL || filename == NULL)
        return ERR_INVALID_PARAMETER;
    zos_dev_t dev = open(filename, O_RDONLY);
    if (dev < 0)
        return -dev;

    err = tracker_read_exact(dev, header, sizeof(header));
    if (err != ERR_SUCCESS)
        goto done;
    if (header[0] != 'Z' || header[1] != 'M' || header[2] != 'T' || header[3] != 0) {
        err = ERR_ENTRY_CORRUPTED;
        goto done;
    }
    zmt_track_init(track);
    err = tracker_read_exact(dev, track->title, TRACKER_TITLE_LEN);
    if (err != ERR_SUCCESS)
        goto done;
    track->title[TRACKER_TITLE_LEN] = 0;
    err = tracker_read_exact(dev, &track->tempo, 1);
    if (err != ERR_SUCCESS)
        goto done;
    if (track->tempo < 4 || track->tempo > 128 || (track->tempo & 3)) {
        err = ERR_ENTRY_CORRUPTED;
        goto done;
    }
    track->current_tempo = track->tempo;
    err = tracker_read_exact(dev, &track->pattern_count, 1);
    if (err != ERR_SUCCESS)
        goto done;
    if (track->pattern_count == 0 || track->pattern_count > NUM_PATTERNS) {
        err = ERR_ENTRY_CORRUPTED;
        goto done;
    }
    for (uint8_t p = 0; p < track->pattern_count; p++) {
        if (track->patterns[p] == NULL) {
            err = ERR_INVALID_PARAMETER;
            goto done;
        }
    }
    err = zmt_arrangement_load(track->arrangement, dev);
    if (err != ERR_SUCCESS)
        goto done;
    for (uint8_t a = 0; a < NUM_ARRANGEMENTS; a++) {
        uint8_t pattern = track->arrangement[a].pattern_index;
        if (pattern != ARRANGEMENT_OUT_OF_RANGE && pattern >= track->pattern_count) {
            err = ERR_ENTRY_CORRUPTED;
            goto done;
        }
    }
    for (uint8_t p = 0; p < track->pattern_count; p++) {
        err = zmt_pattern_load(track->patterns[p], dev);
        if (err != ERR_SUCCESS)
            goto done;
    }
done:
    {
        zos_err_t close_err = close(dev);
        return err == ERR_SUCCESS ? close_err : err;
    }
}
