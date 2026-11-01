#include "zgdk/tilemap.h"

Tilemap *_TILEMAP;

zos_err_t tilemap_register(Tilemap *tilemap, uint8_t *tiles) {
    _TILEMAP=tilemap;
    _TILEMAP->tiles = tiles;
    return ERR_SUCCESS;
}
