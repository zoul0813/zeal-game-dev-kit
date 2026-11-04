#include <zos_vfs.h>
#include "zgdk/utils/ensure_parent_dirs.h"
#include "internal.h"

zos_err_t hiscore_save(const char *path) {
    zos_err_t err = ensure_parent_dirs(path);
    if(err != ERR_SUCCESS)
        return err;

    zos_dev_t dev = open(path, O_WRONLY | O_CREAT | O_TRUNC);
    if(dev < 0)
        return (zos_err_t)(-dev);

    uint16_t size = 3;
    err = write(dev, HISCORE_FORMAT, &size);
    if(err != ERR_SUCCESS)
        goto done;
    if(size != 3) {
        err = ERR_FAILURE;
        goto done;
    }

    uint8_t version = HISCORE_VERSION;
    size = sizeof(version);
    err = write(dev, &version, &size);
    if(err != ERR_SUCCESS)
        goto done;
    if(size != sizeof(version)) {
        err = ERR_FAILURE;
        goto done;
    }

    size = sizeof(hiscore_table);
    err = write(dev, hiscore_table, &size);
    if(err != ERR_SUCCESS)
        goto done;
    if(size != sizeof(hiscore_table)) {
        err = ERR_FAILURE;
        goto done;
    }

    size = sizeof(hiscore_last_initials);
    err = write(dev, hiscore_last_initials, &size);
    if(err != ERR_SUCCESS)
        goto done;
    if(size != sizeof(hiscore_last_initials))
        err = ERR_FAILURE;

done:
    close(dev);
    return err;
}
