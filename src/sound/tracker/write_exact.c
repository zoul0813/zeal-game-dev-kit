#include "private.h"

zos_err_t tracker_write_exact(zos_dev_t dev, const void *data, uint16_t size)
{
    const uint8_t *pos = data;
    while (size != 0) {
        uint16_t written = size;
        zos_err_t err = write(dev, pos, &written);
        if (err != ERR_SUCCESS) return err;
        if (written == 0 || written > size) return ERR_FAILURE;
        pos += written;
        size -= written;
    }
    return ERR_SUCCESS;
}
