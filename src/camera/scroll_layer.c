#include "zgdk/camera.h"
#include "zgdk/tilemap/scroll.h"
#include "wrap.h"

/* origin is kept mod period << CAMERA_MAX_SHIFT, so (x + origin) >> shift has
 * the same remainder as the world position >> shift, and fits 16 bits. */
zos_err_t camera_scroll_layer(const camera_t *c, uint8_t layer, uint8_t sx, uint8_t sy) {
    if (!c || layer > 1 || sx > CAMERA_MAX_SHIFT || sy > CAMERA_MAX_SHIFT) return ERR_INVALID_PARAMETER;
    tilemap_scroll(layer, camera_wrap(((uint16_t)c->x + c->origin_x) >> sx, 1280),
                          camera_wrap(((uint16_t)c->y + c->origin_y) >> sy, 640));
    return ERR_SUCCESS;
}
