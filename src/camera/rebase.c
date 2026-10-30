#include "zgdk/camera.h"

/* Keep the ordinary remainder and count whole hardware periods separately.
 * 32768 periods preserve every supported parallax shift, and all update
 * arithmetic fits 16 bits even when delta is UINT16_MAX. */
static void rebase_phase(uint16_t *origin, uint32_t *phase, uint16_t delta, uint16_t period) {
    uint16_t remainder = *origin + delta % period;
    uint16_t turns = delta / period;
    if (remainder >= period) { remainder -= period; turns++; }
    *origin = remainder;
    *phase = ((uint16_t)*phase + turns) & 32767U;
}

/* Flipping the sign bit gives the unsigned distance to INT16_MIN. */
zos_err_t camera_rebase(camera_t *c, uint16_t dx, uint16_t dy) {
    if (!c || dx > ((uint16_t)c->x ^ 0x8000U) || dy > ((uint16_t)c->y ^ 0x8000U))
        return ERR_INVALID_PARAMETER;
    c->x -= dx; c->y -= dy;
    rebase_phase(&c->origin_x, &c->phase_x, dx, 1280);
    rebase_phase(&c->origin_y, &c->phase_y, dy, 640);
    return ERR_SUCCESS;
}
