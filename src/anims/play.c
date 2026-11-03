#include "zgdk/anims/anim.h"

void anim_play(anim_t *anim, const anim_def *def) {
    if (anim->def != def) anim_restart(anim, def);
}
