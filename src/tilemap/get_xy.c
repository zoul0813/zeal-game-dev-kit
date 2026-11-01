#include "zgdk/tilemap.h"

int16_t tilemap_get_xy(uint8_t x, uint8_t y) {
    if(_TILEMAP == NULL) return -1;
    if(x >= _TILEMAP->rect.w) return -2;
    if(y >= _TILEMAP->rect.h) return -3;

    uint16_t offset = ((y * _TILEMAP->rect.w) + x);
    return _TILEMAP->tiles[offset];
}
