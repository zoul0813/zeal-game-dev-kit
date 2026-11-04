#ifndef ZGDK_PHYSICS_INERTIAL_H
#define ZGDK_PHYSICS_INERTIAL_H
#include <zgdk/physics/body.h>

/* Optional eight-way velocity controller: no gravity, collision, rendering or input
 * device dependency. All values are 12.4 per tick. accel, drag, max_speed and
 * glide must be nonnegative; keep velocity sums within int16_t range.
 * Zero drag/glide preserves momentum; zero drift_y suits top-down/space movement. */
typedef struct {
    int16_t accel;      /* acceleration on a held axis */
    int16_t drag;       /* approach idle velocity when that axis has no input */
    int16_t max_speed;  /* per-axis cap, reduced to 3/4 for diagonal input */
    int16_t drift_y;    /* idle vertical target, clamped to speed cap; negative rises */
    int16_t glide;      /* approach speed cap after a faster external impulse */
} inertial_config_t;

/* dir_x/dir_y: -1, 0, 1. Updates velocity only; call body_integrate() for free
 * movement or body_step() for tile collision afterwards. No controller state.
 * While overspeed, an axis glides toward its cap before accepting input again. */
void inertial_update(body_t *body, const inertial_config_t *config, int8_t dir_x, int8_t dir_y);
#endif
