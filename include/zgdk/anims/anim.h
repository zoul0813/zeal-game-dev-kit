#ifndef ZGDK_ANIMS_ANIM_H
#define ZGDK_ANIMS_ANIM_H

#include <stdint.h>
#include <stddef.h>
#include <zgdk/inline.h>

#define ANIM_LOOP  0x01

/**
 * @brief A list of tiles shown `rate` frames each
 */
typedef struct {
    const uint8_t *frames;
    uint8_t count;
    uint8_t rate;
    uint8_t flags;
} anim_def;

typedef struct {
    const anim_def *def;
    uint8_t index;
    uint8_t timer;
    uint8_t done;   // set after a non-looping animation consumed its last frame
} anim_t;

/**
 * @brief Start `def`, unless it is already playing
 */
void anim_play(anim_t *anim, const anim_def *def);

/**
 * @brief Start `def` from its first frame even if it is already playing
 */
void anim_restart(anim_t *anim, const anim_def *def);

/**
 * @brief Consume one update tick and prepare playback for the next update
 * @return tile for this update, before advancing playback
 */
uint8_t anim_tick(anim_t *anim);

INLINE uint8_t anim_tile(const anim_t *anim) {
    return anim->def ? anim->def->frames[anim->index] : 0;
}

#endif
