#include <core.h>
#include "zgdk/sound/tracker.h"
#include "zgdk/utils/log.h"
#include "private.h"

zos_err_t zmt_file_load(track_t* track, const char* filename)
{
    zos_err_t err;
    uint16_t size = 0;
    char textbuff[TRACKER_TITLE_LEN + 1];

    zmt_track_init(track); // clear out the track, before loading data

    zos_dev_t file_dev = open(filename, O_RDONLY);
    if (file_dev < 0) {
        tracker_log_open_error("failed to open file", filename, (zos_err_t)(-file_dev));
        return -file_dev;
    }

    // printf("Loading '%s' ...\n", filename);

    size = 3;
    err  = read(file_dev, textbuff, &size); // format header
    if (err != ERR_SUCCESS) {
        log_error("error reading format header", err);
        return err;
    }
    // printf("Format: %.3s\n", textbuff);

    size = sizeof(uint8_t);
    err  = read(file_dev, textbuff, &size); // version header
    if (err != ERR_SUCCESS) {
        log_error("error reading version header", err);
        return err;
    }
    // printf("Version: %d\n", textbuff[0]);

    size = TRACKER_TITLE_LEN;
    err  = read(file_dev, textbuff, &size); // track title
    if (err != ERR_SUCCESS) {
        log_error("error reading track title", err);
        return err;
    }
    mem_cpy(track->title, textbuff, size);
    track->title[TRACKER_TITLE_LEN] = 0x00; // NUL
    // printf("Track: %12s (read: %d)\n", track->title, size);

    size = sizeof(uint8_t);
    err  = read(file_dev, &track->tempo, &size);
    if (err != ERR_SUCCESS) {
        log_error("error reading tempo", err);
        return err;
    }

    track->current_tempo = track->tempo;

    size = sizeof(uint8_t);
    err  = read(file_dev, &track->pattern_count, &size); // pattern count
    if (err != ERR_SUCCESS) {
        log_error("error reading pattern count", err);
        return err;
    }
    // printf("Patterns: %d\n", track->pattern_count);

    err = zmt_arrangement_load(track->arrangement, file_dev);
    if (err != ERR_SUCCESS) {
        log_error("error reading arrangement", err);
        return err;
    }

    for (uint8_t p = 0; p < track->pattern_count; p++) {
        // printf("Loading pattern %d\n", p);
        err = zmt_pattern_load(track->patterns[p], file_dev);
        if (err != ERR_SUCCESS) {
            log_error("error loading patterns", err);
            return err;
        }
    }

    // printf("File loaded.\n\n");
    err = close(file_dev);
    return err;
}
