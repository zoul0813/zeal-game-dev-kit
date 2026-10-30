#include "zgdk/camera.h"
#include <string.h>

zos_err_t camera_init_view(camera_t *c, uint16_t ww, uint16_t wh,
                           uint16_t vw, uint16_t vh) {
    /* 1..CAMERA_MAX_SIZE each: one unsigned compare catches 0 too */
    if (!c || (uint16_t)(ww - 1) >= CAMERA_MAX_SIZE || (uint16_t)(wh - 1) >= CAMERA_MAX_SIZE ||
        (uint16_t)(vw - 1) >= CAMERA_MAX_SIZE || (uint16_t)(vh - 1) >= CAMERA_MAX_SIZE)
        return ERR_INVALID_PARAMETER;
    memset(c, 0, sizeof(*c));
    c->max_x = ww > vw ? ww - vw : 0;
    c->max_y = wh > vh ? wh - vh : 0;
    c->view_w = vw; c->view_h = vh;
    c->dz_left = c->dz_right = vw / 3;
    /* 3 * vh fits 16 bits for vh <= CAMERA_MAX_SIZE */
    c->dz_top = c->dz_bottom = (vh * 3) >> 3;
    return ERR_SUCCESS;
}
