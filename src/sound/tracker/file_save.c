#include <core.h>
#include "zgdk/sound/tracker.h"
#include "zgdk/utils/log.h"
#include "private.h"

/* save a track to file */
zos_err_t zmt_file_save(track_t* track, const char* filename)
{
    zos_err_t err;
    uint16_t size = 0;
    char textbuff[TRACKER_TITLE_LEN + 1];
    zos_dev_t file_dev = open(filename, O_WRONLY | O_CREAT | O_TRUNC);
    if (file_dev < 0) {
        tracker_log_open_error("failed to open file for saving", filename, (zos_err_t)(-file_dev));
        return -file_dev;
    }

    /** FILE HEADER */
    size = 3;
    err  = tracker_write_exact(file_dev, "ZMT", size);
    if (err != ERR_SUCCESS) {
        log_error("error saving format header", err);
        goto done;
    }

    size        = sizeof(uint8_t);
    textbuff[0] = 0;
    err         = tracker_write_exact(file_dev, textbuff, size);
    if (err != ERR_SUCCESS) {
        log_error("error saving version header", err);
        goto done;
    }

    size = TRACKER_TITLE_LEN;
    tracker_copy_title_field(textbuff, track->title);
    err = tracker_write_exact(file_dev, textbuff, size);
    if (err != ERR_SUCCESS) {
        log_error("error saving title length", err);
        goto done;
    }

    size = sizeof(uint8_t);
    err  = tracker_write_exact(file_dev, &track->tempo, size);
    if (err != ERR_SUCCESS) {
        log_error("error saving tempo", err);
        goto done;
    }

    size = sizeof(uint8_t);
    err  = tracker_write_exact(file_dev, &track->pattern_count, size);
    if (err != ERR_SUCCESS) {
        log_error("error saving pattern count", err);
        goto done;
    }

    err = zmt_arrangement_save(track->arrangement, file_dev);
    if (err != ERR_SUCCESS) {
        log_error("error saving arrangement", err);
        goto done;
    }

    for (uint8_t p = 0; p < track->pattern_count; p++) {
        err = zmt_pattern_save(track->patterns[p], file_dev);
        if (err != ERR_SUCCESS) {
            log_error("error saving patterns", err);
            goto done;
        }
    }

done:
    {
        zos_err_t close_err = close(file_dev);
        return err == ERR_SUCCESS ? close_err : err;
    }
}
