#ifndef ZGDK_PHYSICS_BODY_H
#define ZGDK_PHYSICS_BODY_H
#include <stdint.h>
#include <zgdk/inline.h>

/* 12.4 fixed point: one pixel = 16 units, range -2048..2047 pixels.
 * Keep positions, velocities and sums in int16_t range. Units are per tick. */
#define PHYSICS_FP_SHIFT 4
#define PHYSICS_FP_ONE (1 << PHYSICS_FP_SHIFT)
#define PHYSICS_FP(px) ((int16_t)((px) * PHYSICS_FP_ONE))
#define PHYSICS_FP_INT(v) ((int16_t)(v) >> PHYSICS_FP_SHIFT)

/* Collision results: ground/ceiling mean positive/negative Y contact.
 * Trigger and ladder are convenience results for standard tile flags. */
#define BODY_ON_GROUND 0x01
#define BODY_HIT_WALL 0x02
#define BODY_HIT_CEIL 0x04
#define BODY_TOUCHING_TRIGGER 0x08
#define BODY_ON_LADDER 0x10
/* Persistent collision policy. Platformer ladders set this while climbing. */
#define BODY_PASS_ONEWAY 0x80

/* Independent of rendering, entities, gravity and controllers: 12 bytes on Z80.
 * x/y are hitbox top-left; w/h are nonzero pixel dimensions (1..255). */
typedef struct {
    int16_t x, y, vx, vy;
    uint8_t w, h, state, touched;
} body_t;

void body_init(body_t *body, int16_t px, int16_t py, uint8_t w, uint8_t h);
/* Teleport in pixels, preserve velocity, clear collision/policy state. */
void body_set_pos(body_t *body, int16_t px, int16_t py);
uint8_t body_overlap(const body_t *a, const body_t *b);

INLINE int16_t body_px(const body_t *body) { return PHYSICS_FP_INT(body->x); }
INLINE int16_t body_py(const body_t *body) { return PHYSICS_FP_INT(body->y); }
/* Free movement: no tilemap/cache/controller linkage. Collision state unchanged. */
INLINE void body_integrate(body_t *body) { body->x += body->vx; body->y += body->vy; }
/* Apply once for an impulse, or each tick for acceleration, in fixed-point units.
 * The caller owns limits; keep arithmetic within int16_t range. */
INLINE void body_accelerate(body_t *body, int16_t ax, int16_t ay) { body->vx += ax; body->vy += ay; }
/* Downward acceleration with a positive terminal speed, both fixed-point. */
INLINE void body_apply_gravity(body_t *body, int16_t gravity, int16_t max_fall) {
    body->vy += gravity;
    if (body->vy > max_fall) body->vy = max_fall;
}
#endif
