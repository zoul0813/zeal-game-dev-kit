#include <zgdk/entities/sprite.h>
#include <core.h>
zos_err_t sprite_entity_init(sprite_entity_t *entity, gfx_sprite *sprite) {
    if (!entity || !sprite) return ERR_INVALID_PARAMETER;
    mem_set(entity, 0, sizeof(*entity));
    mem_set(sprite, 0, sizeof(*sprite));
    entity->sprite = sprite;
    entity->parked = 1;
    return ERR_SUCCESS;
}
