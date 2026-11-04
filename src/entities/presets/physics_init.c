#include <zgdk/entities/physics.h>
zos_err_t physics_entity_init(physics_entity_t *entity, gfx_sprite *sprite,
    int16_t px, int16_t py, uint8_t w, uint8_t h) {
    zos_err_t err;
    if (!entity || !w || !h) return ERR_INVALID_PARAMETER;
    err = sprite_entity_init(&entity->visual, sprite);
    if (err == ERR_SUCCESS) body_init(&entity->body, px, py, w, h);
    return err;
}
