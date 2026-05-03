#include <zos_errors.h>
#include <zos_vfs.h>

zos_err_t ensure_parent_dirs(const char *path) {
    char dir[PATH_MAX];
    uint8_t i = 0;

    while(path[i] != 0) {
        if(i >= PATH_MAX - 1)
            return ERR_PATH_TOO_LONG;
        dir[i] = path[i];
        i++;
    }
    dir[i] = 0;

    for(i = 0; dir[i] != 0; i++) {
        if(dir[i] != '/')
            continue;

        if(i == 0)
            continue;
        if((i == 2) && (dir[1] == ':'))
            continue;

        dir[i] = 0;
        zos_err_t err = mkdir(dir);
        dir[i] = '/';

        if((err != ERR_SUCCESS) && (err != ERR_ALREADY_EXIST))
            return err;
    }

    return ERR_SUCCESS;
}
