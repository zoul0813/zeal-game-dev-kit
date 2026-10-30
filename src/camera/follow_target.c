#include "zgdk/camera.h"

/* Separate object: manual cameras do not link following. Wider arithmetic also
 * handles negative targets and unsigned target dimensions at coordinate limits. */
void camera_follow_target(camera_t *c, int16_t tx, int16_t ty,
                          uint16_t w, uint16_t h, int8_t direction) {
    int32_t left = tx, top = ty, x = c->x, y = c->y;
    int32_t right = (int32_t)c->view_w - c->dz_right;
    int32_t bottom = (int32_t)c->view_h - c->dz_bottom;
    if (direction > 0) left += c->lead;
    else if (direction < 0) left -= c->lead;
    if (left < x + c->dz_left) x = left - c->dz_left;
    else if (left + w > x + right) x = left + w - right;
    if (top < y + c->dz_top) y = top - c->dz_top;
    else if (top + h > y + bottom) y = top + h - bottom;
    /* Project the target into the world before limiting the step, then clamp
     * again in camera_set. Projection before and after a bounded step gives
     * the same final position, including temporarily negative rebased cameras.
     * It lets step arithmetic stay 16-bit after the wide target calculations. */
    if (x < 0) x = 0; else if (x > c->max_x) x = c->max_x;
    if (y < 0) y = 0; else if (y > c->max_y) y = c->max_y;
    {
        int16_t nx = (int16_t)x, ny = (int16_t)y;
        uint8_t step = c->max_step;
        if (step) {
            if (nx > c->x && (uint16_t)((uint16_t)nx - (uint16_t)c->x) > step) nx = c->x + step;
            else if (nx < c->x && (uint16_t)((uint16_t)c->x - (uint16_t)nx) > step) nx = c->x - step;
            if (ny > c->y && (uint16_t)((uint16_t)ny - (uint16_t)c->y) > step) ny = c->y + step;
            else if (ny < c->y && (uint16_t)((uint16_t)c->y - (uint16_t)ny) > step) ny = c->y - step;
        }
        camera_set(c, nx, ny);
    }
}
