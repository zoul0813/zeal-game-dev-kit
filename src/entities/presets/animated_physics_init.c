#include <zgdk/entities/animated_physics.h>
zos_err_t animated_physics_entity_init(animated_physics_entity_t *entity, gfx_sprite *sprite,
    int16_t px, int16_t py, uint8_t w, uint8_t h) {
    zos_err_t err;
    if (!entity) return ERR_INVALID_PARAMETER;
    err = physics_entity_init(&entity->physics, sprite, px, py, w, h);
    if (err == ERR_SUCCESS) anim_restart(&entity->anim, NULL);
    return err;
}
