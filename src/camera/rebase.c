#include "zgdk/camera.h"
#include "wrap.h"

/* The local position stays 0..32767 and origin below 1280 << 4, so the sums fit 16 bits. */
zos_err_t camera_rebase(camera_t *c, uint16_t dx, uint16_t dy) {
    if (!c || c->x < 0 || c->y < 0 || dx > (uint16_t)c->x || dy > (uint16_t)c->y)
        return ERR_INVALID_PARAMETER;
    c->x -= dx; c->y -= dy;
    c->origin_x = camera_wrap(c->origin_x + dx, 1280U << CAMERA_MAX_SHIFT);
    c->origin_y = camera_wrap(c->origin_y + dy, 640U << CAMERA_MAX_SHIFT);
    return ERR_SUCCESS;
}
