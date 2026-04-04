#include "zgdk/sound/tracker.h"
#include "private.h"

zos_err_t zmt_arrangement_load(arrangement_t arrangement[NUM_ARRANGEMENTS], zos_dev_t dev)
{
    zos_err_t err = ERR_SUCCESS;
    uint16_t size = 0;
    uint8_t i     = 0;
    arrangement_t* a;

    uint32_t bitmap_high = 0;
    uint32_t bitmap_low  = 0;
    uint8_t bit          = 0;

    size = sizeof(uint32_t);
    err  = read(dev, &bitmap_low, &size);
    if (err != ERR_SUCCESS) {
        tracker_log_io_error("write", 26, err);
        return err;
    }
    size = sizeof(uint32_t);
    err  = read(dev, &bitmap_high, &size);
    if (err != ERR_SUCCESS) {
        tracker_log_io_error("write", 27, err);
        return err;
    }

    for (i = 0; i < NUM_ARRANGEMENTS / 2; i++) {
        bit        = bitmap_low & 0x01;
        bitmap_low = bitmap_low >> 1;
        if (bit == 0)
            continue;
        a    = &arrangement[i];
        size = sizeof(arrangement_t);
        err  = read(dev, a, &size);
        if (err != ERR_SUCCESS) {
            tracker_log_io_error("write", 18, err);
            return err;
        }
    }
    for (i = NUM_ARRANGEMENTS / 2; i < NUM_ARRANGEMENTS; i++) {
        bit         = bitmap_high & 0x01;
        bitmap_high = bitmap_high >> 1;
        if (bit == 0)
            continue;
        a    = &arrangement[i];
        size = sizeof(arrangement_t);
        err  = read(dev, a, &size);
        if (err != ERR_SUCCESS) {
            tracker_log_io_error("write", 18, err);
            return err;
        }
    }

    return err;
}
