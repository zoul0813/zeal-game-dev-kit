#include "zgdk/tilemap.h"

int16_t tilemap_get(Point *point) {
    // offset to map space
    Point map_point;
    vec2_copy_pointer(&map_point, point);
    map_point.x -= (_TILEMAP->rect.x);
    map_point.y -= (_TILEMAP->rect.y);

    return tilemap_get_xy(map_point.x, map_point.y);
}
