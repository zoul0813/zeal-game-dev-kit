#include <zgdk/entities/animated.h>
zos_err_t animated_entity_init(animated_entity_t *entity, gfx_sprite *sprite) {
    zos_err_t err;
    if (!entity) return ERR_INVALID_PARAMETER;
    err = sprite_entity_init(&entity->visual, sprite);
    if (err == ERR_SUCCESS) anim_restart(&entity->anim, NULL);
    return err;
}
