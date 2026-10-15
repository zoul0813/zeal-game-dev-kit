#include "private.h"

zos_err_t tracker_write_exact(zos_dev_t dev, const void* buffer, uint16_t length)
{
    uint16_t size = length;
    zos_err_t err = write(dev, buffer, &size);
    if (err != ERR_SUCCESS)
        return err;
    return size == length ? ERR_SUCCESS : ERR_FAILURE;
}
