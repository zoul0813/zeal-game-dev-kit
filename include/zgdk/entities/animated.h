#ifndef ZGDK_ENTITIES_ANIMATED_H
#define ZGDK_ENTITIES_ANIMATED_H
#include <zgdk/entities/sprite.h>
#include <zgdk/anims/anim.h>
typedef struct { sprite_entity_t visual; anim_t anim; } animated_entity_t;
#ifdef __SDCC
_Static_assert(sizeof(animated_entity_t) == 14, "animated entity layout");
#endif
zos_err_t animated_entity_init(animated_entity_t *entity, gfx_sprite *sprite);
/* Selection/restart also seeds the first tile. Repeated selection holds the last
 * returned tile. Tick explicitly: e->visual.sprite->tile = anim_tick(&e->anim). */
void animated_entity_play(animated_entity_t *entity, const anim_def *def);
void animated_entity_restart(animated_entity_t *entity, const anim_def *def);
#endif
