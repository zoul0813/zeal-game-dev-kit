#include "zgdk/anims/anim.h"

uint8_t anim_tick(anim_t *anim) {
    const anim_def *def = anim->def;
    uint8_t index;
    if (def == NULL) return 0;
    index = anim->index;
    if (!anim->done && ++anim->timer >= def->rate) {
        anim->timer = 0;
        uint8_t next = index + 1;
        if (next < def->count) {
            anim->index = next;
        } else if (def->flags & ANIM_LOOP) {
            anim->index = 0;
        } else {
            anim->done = 1;
        }
    }
    return def->frames[index];
}
