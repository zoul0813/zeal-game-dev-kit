#ifndef ZGDK_ENTITIES_ANIMATED_PHYSICS_H
#define ZGDK_ENTITIES_ANIMATED_PHYSICS_H
#include <zgdk/entities/physics.h>
#include <zgdk/anims/anim.h>
typedef struct { physics_entity_t physics; anim_t anim; } animated_physics_entity_t;
#ifdef __SDCC
_Static_assert(sizeof(animated_physics_entity_t) == 26, "animated physics entity layout");
#endif
zos_err_t animated_physics_entity_init(animated_physics_entity_t *entity, gfx_sprite *sprite,
    int16_t px, int16_t py, uint8_t w, uint8_t h);
void animated_physics_entity_play(animated_physics_entity_t *entity, const anim_def *def);
void animated_physics_entity_restart(animated_physics_entity_t *entity, const anim_def *def);
/* Tick explicitly into e->physics.visual.sprite->tile. Use physics_entity_sync
 * or physics_entities_sync; animation never forces physics to step. */
#endif
