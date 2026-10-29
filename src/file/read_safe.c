#include "zgdk/file.h"

zos_err_t file_read_safe(zos_dev_t dev, void *buffer, uint16_t *size)
{
    uint8_t *ptr = (uint8_t *)buffer;
    uint16_t left = *size;
    uint16_t room;
    uint16_t want;
    uint16_t got;
    zos_err_t err = ERR_SUCCESS;

    while (left > 0) {
        // the rest of this 16 KiB page
        room = 0x4000 - (uint16_t)((uintptr_t)ptr & 0x3FFF);
        want = left < room ? left : room;
        got = want;
        err = read(dev, ptr, &got);
        if (err != ERR_SUCCESS) break;
        ptr += got;
        left -= got;
        // fewer bytes than asked: the end of the file
        if (got < want) break;
    }
    *size -= left;
    return err;
}
