/* Two typed implementations share source logic without runtime forwarding or
 * extra component-address arguments. Each preset is a separate archive member. */
#define ENTITY_ANIMATION(prefix, type, visual) \
void prefix##_restart(type *entity, const anim_def *def) { \
    anim_restart(&entity->anim, def); \
    entity->visual.sprite->tile = def ? def->frames[0] : 0; \
} \
void prefix##_play(type *entity, const anim_def *def) { \
    if (entity->anim.def != def) prefix##_restart(entity, def); \
}
