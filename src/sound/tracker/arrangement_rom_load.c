#include <core.h>
#include "zgdk/sound/tracker.h"

zos_err_t zmt_arrangement_rom_load(arrangement_t arrangement[NUM_ARRANGEMENTS], uint8_t **start)
{
    uint8_t *pos = *start;
    zos_err_t err = ERR_SUCCESS;
    uint16_t size = 0;
    uint8_t i     = 0;
    arrangement_t* a;

    uint32_t bitmap_high = 0;
    uint32_t bitmap_low  = 0;
    uint8_t bit          = 0;

    size = sizeof(uint32_t);
    mem_cpy(&bitmap_low, pos, size);
    pos += size;

    size = sizeof(uint32_t);
    mem_cpy(&bitmap_high, pos, size);
    pos += size;


    for (i = 0; i < NUM_ARRANGEMENTS / 2; i++) {
        bit        = bitmap_low & 0x01;
        bitmap_low = bitmap_low >> 1;
        if (bit == 0)
            continue;
        a    = &arrangement[i];
        size = sizeof(arrangement_t);
        mem_cpy(a, pos, size);
        pos += size;
    }
    for (i = NUM_ARRANGEMENTS / 2; i < NUM_ARRANGEMENTS; i++) {
        bit         = bitmap_high & 0x01;
        bitmap_high = bitmap_high >> 1;
        if (bit == 0)
            continue;
        a    = &arrangement[i];
        size = sizeof(arrangement_t);
        mem_cpy(a, pos, size);
        pos += size;
    }

    *start = pos;
    return err;
}
