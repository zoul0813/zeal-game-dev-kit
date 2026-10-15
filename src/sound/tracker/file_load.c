#include <core.h>
#include <zos_sys.h>
#include "zgdk/sound/tracker.h"
#include "private.h"

/* Keep the old composition outside the program's 48KB address space. */
__sfr __banked __at(0xF0) tracker_mmu_page0;
#define BACKUP_BASE ((uint8_t*)0x0100)
typedef char backup_fits_page[(0x100 + sizeof(track_t) + NUM_PATTERNS * sizeof(pattern_t) <= 0x4000) ? 1 : -1];

static void copy_backup(track_t* track, uint8_t page, uint8_t restore) __critical
{
    uint8_t previous = tracker_mmu_page0;
    tracker_mmu_page0 = page;
    if (restore)
        mem_cpy(track, BACKUP_BASE, sizeof(track_t));
    else
        mem_cpy(BACKUP_BASE, track, sizeof(track_t));
    for (uint8_t p = 0; p < NUM_PATTERNS; p++) {
        uint8_t* saved = BACKUP_BASE + sizeof(track_t) + p * sizeof(pattern_t);
        if (track->patterns[p] != NULL) {
            if (restore)
                mem_cpy(track->patterns[p], saved, sizeof(pattern_t));
            else
                mem_cpy(saved, track->patterns[p], sizeof(pattern_t));
        }
    }
    tracker_mmu_page0 = previous;
}

static zos_err_t load_track(track_t* track, zos_dev_t dev)
{
    zos_err_t err;
    uint8_t header[4];
    err = tracker_read_exact(dev, header, sizeof(header));
    if (err != ERR_SUCCESS)
        return err;
    if (header[0] != 'Z' || header[1] != 'M' || header[2] != 'T' || header[3] != 0) {
        err = ERR_ENTRY_CORRUPTED;
        return err;
    }
    err = tracker_read_exact(dev, track->title, TRACKER_TITLE_LEN);
    if (err != ERR_SUCCESS)
        return err;
    track->title[TRACKER_TITLE_LEN] = 0;
    err = tracker_read_exact(dev, &track->tempo, 1);
    if (err != ERR_SUCCESS)
        return err;
    if (track->tempo < 4 || track->tempo > 128 || (track->tempo & 3)) {
        err = ERR_ENTRY_CORRUPTED;
        return err;
    }
    track->current_tempo = track->tempo;
    err = tracker_read_exact(dev, &track->pattern_count, 1);
    if (err != ERR_SUCCESS)
        return err;
    if (track->pattern_count == 0 || track->pattern_count > NUM_PATTERNS) {
        err = ERR_ENTRY_CORRUPTED;
        return err;
    }
    for (uint8_t p = 0; p < track->pattern_count; p++) {
        if (track->patterns[p] == NULL) {
            err = ERR_INVALID_PARAMETER;
            return err;
        }
    }
    err = zmt_arrangement_load(track->arrangement, dev);
    if (err != ERR_SUCCESS)
        return err;
    for (uint8_t a = 0; a < NUM_ARRANGEMENTS; a++) {
        uint8_t pattern = track->arrangement[a].pattern_index;
        if (pattern != ARRANGEMENT_OUT_OF_RANGE && pattern >= track->pattern_count) {
            err = ERR_ENTRY_CORRUPTED;
            return err;
        }
    }
    for (uint8_t p = 0; p < track->pattern_count; p++) {
        err = zmt_pattern_load(track->patterns[p], dev);
        if (err != ERR_SUCCESS)
            return err;
    }
    return ERR_SUCCESS;
}

zos_err_t zmt_file_load(track_t* track, const char* filename)
{
    if (track == NULL || filename == NULL)
        return ERR_INVALID_PARAMETER;
    zos_dev_t dev = open(filename, O_RDONLY);
    if (dev < 0)
        return -dev;
    uint8_t page;
    zos_err_t err = palloc(&page);
    if (err != ERR_SUCCESS) {
        close(dev);
        return err;
    }
    copy_backup(track, page, 0);
    err = load_track(track, dev);
    zos_err_t close_err = close(dev);
    if (err == ERR_SUCCESS)
        err = close_err;
    if (err != ERR_SUCCESS)
        copy_backup(track, page, 1);
    /* A valid allocated page remains mapped only inside copy_backup. */
    zos_err_t free_err = pfree(page);
    if (err == ERR_SUCCESS && free_err != ERR_SUCCESS) {
        copy_backup(track, page, 1);
        return free_err;
    }
    if (err == ERR_SUCCESS) {
        zmt_current_pattern = 0;
        zmt_track_reset(track, 0);
    }
    return err;
}
