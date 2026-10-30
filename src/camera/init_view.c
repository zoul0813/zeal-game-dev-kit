#include "zgdk/camera.h"
#include <string.h>

zos_err_t camera_init_view(camera_t *c, uint16_t ww, uint16_t wh,
                           uint16_t vw, uint16_t vh) {
    if (!c || !ww || !wh || !vw || !vh) return ERR_INVALID_PARAMETER;
    ww = ww > vw ? ww - vw : 0;
    wh = wh > vh ? wh - vh : 0;
    if (ww > 32767 || wh > 32767) return ERR_INVALID_PARAMETER;
    memset(c, 0, sizeof(*c));
    c->max_x = ww;
    c->max_y = wh;
    c->view_w = vw; c->view_h = vh;
    c->dz_left = c->dz_right = vw / 3;
    /* floor(3*vh/8) without overflowing at UINT16_MAX. */
    c->dz_top = c->dz_bottom = (vh / 8) * 3 + (vh % 8) * 3 / 8;
    return ERR_SUCCESS;
}
