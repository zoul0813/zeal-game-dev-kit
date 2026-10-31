#include <stdint.h>
#include <stddef.h>
#include <zvb_hardware.h>
#include <zvb_sound.h>
#include "zgdk/sound/sfx.h"

static const sfx_seg *current;   // NULL when idle
static uint8_t frame;
static int32_t DIV, STEP;    // 16.16 hardware divider; divides once per segment, never per frame

static uint8_t PRIORITY;
static sound_voice_t MASK;
static sound_volume_t VOLUME;

/* ZVB's zvb_sound_* calls map the sound peripheral without restoring the previous page,
 * so every call here saves and restores it (as ZGDK's sounds.c and the tracker do). */
/* writes the divider directly: ZGDK's sound_play() takes Hz and does a 32-bit divide per call */
static void voice_out(uint16_t divider, uint8_t wave) {
    uint8_t page = zvb_config_dev_idx;
    zvb_sound_set_voices(MASK, divider, wave | DUTY_CYCLE_50_0);
    zvb_map_peripheral(page);
}

static void voice_volume(sound_volume_t vol) {
    uint8_t page = zvb_config_dev_idx;
    zvb_sound_set_voices_vol(MASK, vol);
    zvb_map_peripheral(page);
}

void sfx_init(sound_voice_t voice_mask, sound_volume_t volume) {
    if (MASK) voice_volume(VOL_0);
    MASK = voice_mask;
    VOLUME = volume;
    current = NULL;
    voice_volume(VOL_0);
}

void sfx_play(const sfx_seg *effect) {
    current = effect;
    frame = 0;
    PRIORITY = 0;
    if (!effect || effect->frames == 0) current = NULL;
    /* The first tick programs the new voice before making it audible. */
    voice_volume(VOL_0);
}

uint8_t sfx_play_prio(const sfx_seg *effect, uint8_t priority) {
    if (!effect || effect->frames == 0 || (current && priority < PRIORITY)) return 0;
    sfx_play(effect);
    PRIORITY = priority;
    return 1;
}

uint8_t sfx_active(void) {
    return current != NULL;
}

void sfx_restore(void) {
    if (current && frame) voice_volume(VOLUME);
}

static void segment_start(void) {
    int32_t to = (int32_t)current->to_div << 16;
    DIV = (int32_t)current->from_div << 16;
    STEP = (to - DIV) / current->frames;
}

void sfx_tick(void) {
    if (current == NULL) return;
    if (frame == 0) segment_start();
    else if (frame >= current->frames) {
        current++;
        frame = 0;
        if (current->frames == 0) {
            current = NULL;
            // a zero divider can't silence noise, so mute the voice instead
            voice_volume(VOL_0);
            return;
        }
        segment_start();
    }
    voice_out((uint16_t)(DIV >> 16), current->wave);
    if (frame == 0) voice_volume(VOLUME);
    DIV += STEP;
    frame++;
}
