#include "zgdk/sound/tracker.h"
#include "private.h"

#define ARRANGEMENT_IS_EMPTY(arrange) \
    ((arrange->pattern_index == ARRANGEMENT_OUT_OF_RANGE) && (arrange->fx == FX_OUT_OF_RANGE))

/* save arrangement to file */
zos_err_t zmt_arrangement_save(arrangement_t arrangement[NUM_ARRANGEMENTS], zos_dev_t dev)
{
    zos_err_t err = ERR_SUCCESS;
    uint16_t size = 0;
    uint8_t i     = 0;
    arrangement_t* a;

    uint32_t bitmap_high = 0;
    uint32_t bitmap_low  = 0;
    uint8_t bitmap_bit   = 0;

    bitmap_bit = 0;

    /* generate bitmap_low */
    for (i = NUM_ARRANGEMENTS; i > NUM_ARRANGEMENTS / 2; i--) {
        a           = &arrangement[i - 1];
        bitmap_high = bitmap_high << 1;
        if (!ARRANGEMENT_IS_EMPTY(a))
            bitmap_high++;
    }
    /* generate bitmap_high */
    for (i = NUM_ARRANGEMENTS / 2; i > 0; i--) {
        a          = &arrangement[i - 1];
        bitmap_low = bitmap_low << 1;
        if (!ARRANGEMENT_IS_EMPTY(a))
            bitmap_low++;
    }

    size = sizeof(uint32_t);
    err  = write(dev, &bitmap_low, &size);
    if (err != ERR_SUCCESS) {
        tracker_log_io_error("write", 16, err);
        return err;
    }

    size = sizeof(uint32_t);
    err  = write(dev, &bitmap_high, &size);
    if (err != ERR_SUCCESS) {
        tracker_log_io_error("write", 17, err);
        return err;
    }

    /* generate arrangement data */
    for (i = 0; i < NUM_ARRANGEMENTS; i++) {
        a = &arrangement[i];
        if (!ARRANGEMENT_IS_EMPTY(a)) {
            size = sizeof(arrangement_t);
            err  = write(dev, a, &size);
            if (err != ERR_SUCCESS) {
                tracker_log_io_error("write", 18, err);
                return err;
            }
        }
    }

    return err;
}
