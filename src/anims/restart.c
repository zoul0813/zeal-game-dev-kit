#include "zgdk/anims/anim.h"

void anim_restart(anim_t *anim, const anim_def *def) {
    anim->def = def;
    anim->index = 0;
    anim->timer = 0;
    anim->done = 0;
}
