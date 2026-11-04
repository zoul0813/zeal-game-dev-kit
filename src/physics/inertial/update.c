#include <zgdk/utils/math.h>
#include <stdint.h>
#include <zgdk/physics/inertial.h>

static int16_t toward(int16_t v, int16_t target, int16_t step) {
    if (v < target) {
        v += step;
        if (v > target) v = target;
    } else if (v > target) {
        v -= step;
        if (v < target) v = target;
    }
    return v;
}

static int16_t axis(int16_t v, int8_t dir, const inertial_config_t *config, int16_t max, int16_t idle) {
    int16_t accel = config->accel;
    if (v > max || v < -max) {
        // faster than the controller cap (an impulse): glide back down, input can't add more
        return toward(v, clamp_i16(v, -max, max), config->glide);
    }
    if (dir > 0) return clamp_i16(v + accel, -max, max);
    if (dir < 0) return clamp_i16(v - accel, -max, max);
    return toward(v, clamp_i16(idle, -max, max), config->drag);
}

void inertial_update(body_t *body, const inertial_config_t *config, int8_t dir_x, int8_t dir_y) {
    int16_t max = config->max_speed;
    // diagonal: 3/4 per axis keeps the speed close to straight swimming (0.707 would be exact)
    if (dir_x && dir_y) max -= max >> 2;
    body->vx = axis(body->vx, dir_x, config, max, 0);
    body->vy = axis(body->vy, dir_y, config, max, config->drift_y);
}
