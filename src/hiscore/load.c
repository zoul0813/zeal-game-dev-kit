#include <core.h>
#include <zos_vfs.h>
#include "internal.h"

zos_err_t hiscore_load(const char *path) {
    char format[3];
    uint8_t version;
    highscore_t loaded_hiscores[HISCORES_COUNT];
    char loaded_initials[3];
    zos_err_t err;
    uint16_t size;

    zos_dev_t dev = open(path, O_RDONLY);
    if(dev < 0)
        return (zos_err_t)(-dev);

    size = 3;
    err = read(dev, format, &size);
    if(err != ERR_SUCCESS)
        goto done;
    if((size != 3) || (mem_cmp(format, HISCORE_FORMAT, 3) != 0)) {
        err = ERR_ENTRY_CORRUPTED;
        goto done;
    }

    size = sizeof(version);
    err = read(dev, &version, &size);
    if(err != ERR_SUCCESS)
        goto done;
    if((size != sizeof(version)) || (version != HISCORE_VERSION)) {
        err = ERR_ENTRY_CORRUPTED;
        goto done;
    }

    size = sizeof(loaded_hiscores);
    err = read(dev, loaded_hiscores, &size);
    if(err != ERR_SUCCESS)
        goto done;
    if(size != sizeof(loaded_hiscores)) {
        err = ERR_ENTRY_CORRUPTED;
        goto done;
    }

    size = sizeof(loaded_initials);
    err = read(dev, loaded_initials, &size);
    if(err != ERR_SUCCESS)
        goto done;
    if(size != sizeof(loaded_initials)) {
        err = ERR_ENTRY_CORRUPTED;
        goto done;
    }

    mem_cpy(hiscore_table, loaded_hiscores, sizeof(hiscore_table));
    mem_cpy(hiscore_last_initials, loaded_initials, sizeof(hiscore_last_initials));

done:
    close(dev);
    return err;
}
