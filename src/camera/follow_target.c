#include "zgdk/camera.h"

/* Separate object: manual cameras do not link following. Within the ranges in
 * camera.h every sum below stays inside -24830..32766, so 16-bit math is exact. */
void camera_follow_target(camera_t *c, int16_t tx, int16_t ty,
                          uint16_t w, uint16_t h, int8_t direction) {
    int16_t x = c->x, y = c->y, step;
    int16_t right = (int16_t)(c->view_w - c->dz_right);
    int16_t bottom = (int16_t)(c->view_h - c->dz_bottom);
    if (direction > 0) tx += c->lead;
    else if (direction < 0) tx -= c->lead;
    if (tx < x + (int16_t)c->dz_left) x = tx - (int16_t)c->dz_left;
    else if (tx + (int16_t)w > x + right) x = tx + (int16_t)w - right;
    if (ty < y + (int16_t)c->dz_top) y = ty - (int16_t)c->dz_top;
    else if (ty + (int16_t)h > y + bottom) y = ty + (int16_t)h - bottom;
    /* limit the step from the current position, then camera_set clamps to the world */
    step = c->max_step;
    if (step) {
        if (x > c->x + step) x = c->x + step;
        else if (x < c->x - step) x = c->x - step;
        if (y > c->y + step) y = c->y + step;
        else if (y < c->y - step) y = c->y - step;
    }
    camera_set(c, x, y);
}
