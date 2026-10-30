#include "zgdk/camera.h"

void camera_set(camera_t *c, int16_t x, int16_t y) {
    if (x > c->max_x) x = c->max_x;
    if (x < 0) x = 0;
    if (y > c->max_y) y = c->max_y;
    if (y < 0) y = 0;
    c->x = x; c->y = y;
}
