#include <stdint.h>
#include <zvb_sound.h>

#ifndef ZGDK_SOUND_SFX_H
#define ZGDK_SOUND_SFX_H

/**
 * Sound effects on one ZGDK voice: a list of segments, each sweeping linearly
 * between two frequencies over `frames` frames. Define segments with SFX_SEG
 * so frequency-to-divider conversion happens at compile time. Ends with SFX_END.
 *
 * Convention: music (ZMT tracks) leaves the sfx voice empty, so effects never cut music.
 * Link zgdk and zvb_sound. This sequencer does not use the simple tone player.
 * One effect is active at a time; the caller retains its segment array until it ends.
 * Nonterminal segments have 1..255 frames and dividers below 32768.
 */
typedef struct {
    uint8_t wave;      // WAV_SQUARE, WAV_TRIANGLE, WAV_SAWTOOTH, WAV_NOISE
    uint16_t from_div;
    uint16_t to_div;
    uint8_t frames;
} sfx_seg;

/* Integer frequencies: 0..22045 Hz, keeping dividers below 32768 for signed 16.16 ramps. */
#define SFX_DIV(hz) ((uint16_t)(((uint32_t)(hz) << 16) / 44091UL))
#define SFX_SEG(wave, from_hz, to_hz, frames) \
    { (wave), SFX_DIV(from_hz), SFX_DIV(to_hz), (frames) }
#define SFX_END { 0, 0, 0, 0 }

/**
 * @brief Select an effect voice with a VOICE0..VOICE3 bitmask, not an index
 *
 * Order matters: sound_init_hardware() (resets the sound chip), then zmt_reset(), then sfx_init().
 * Mutes the previously selected voice, if any. The new voice starts muted.
 */
void sfx_init(sound_voice_t voice_mask, sound_volume_t volume);

/**
 * @brief Start an effect, replacing the one playing
 *
 * NULL or an empty list (SFX_END first) stops the current effect and mutes its voice.
 * Playback becomes audible on the next sfx_tick(), after programming the voice.
 */
void sfx_play(const sfx_seg *effect);

/**
 * @brief Start an effect unless a higher-priority one is playing
 *
 * Plays when idle or when `priority` >= the playing effect's priority.
 * sfx_play() is priority 0 and always replaces.
 * NULL or empty requests return 0 without interrupting the current effect.
 * @return 1 if the effect started
 */
uint8_t sfx_play_prio(const sfx_seg *effect, uint8_t priority);

/**
 * @brief Advance the current effect, call once per frame
 */
void sfx_tick(void);

/**
 * @brief Is an effect playing?
 */
uint8_t sfx_active(void);

/**
 * @brief Restore the effect voice volume, ie; after zmt_sound_off() muted every voice
 */
void sfx_restore(void);

#endif
