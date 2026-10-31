#include <assert.h>
#include <stdio.h>
#include "zgdk/sound/sfx.h"
#include "zgdk/sound/sounds.h"

uint8_t zvb_config_dev_idx;
static uint16_t played[256];
static unsigned n, resets;
static sound_voice_t last_mask;
static sound_volume_t voice_volume, master_volume;
static sound_waveform_t last_wave;
static uint8_t held;
static uint16_t dividers[MAX_VOICES], unmuted_dividers[MAX_VOICES];
static sound_waveform_t waves[MAX_VOICES], unmuted_waves[MAX_VOICES];
static sound_volume_t volumes[MAX_VOICES];

void zvb_sound_initialize(uint8_t reset) {
    assert(reset == 1); resets++;
    zvb_map_peripheral(ZVB_PERI_SOUND_IDX);
}
void zvb_sound_set_voices(sound_voice_t mask, uint16_t divider, sound_waveform_t wave) {
    assert(n < 256); played[n++] = divider;
    last_mask = mask; last_wave = wave;
    for (unsigned i = 0; i < MAX_VOICES; ++i)
        if (mask & (1U << i)) { dividers[i] = divider; waves[i] = wave; }
    zvb_map_peripheral(ZVB_PERI_SOUND_IDX);
}
void zvb_sound_set_voices_vol(sound_voice_t mask, sound_volume_t volume) {
    last_mask = mask; voice_volume = volume;
    for (unsigned i = 0; i < MAX_VOICES; ++i) {
        if (!(mask & (1U << i))) continue;
        volumes[i] = volume;
        if (volume != VOL_0) {
            unmuted_dividers[i] = dividers[i];
            unmuted_waves[i] = waves[i];
        }
    }
    zvb_map_peripheral(ZVB_PERI_SOUND_IDX);
}
void zvb_sound_set_hold(sound_voice_t mask, uint8_t value) {
    assert(mask == VOICEALL); held = value;
    zvb_map_peripheral(ZVB_PERI_SOUND_IDX);
}
void zvb_sound_set_volume(sound_volume_t volume) {
    master_volume = volume;
    zvb_map_peripheral(ZVB_PERI_SOUND_IDX);
}

int main(void) {
    static const uint16_t hz[] = {0, 90, 260, 620, 900, 3000, 6000, 7000, 22045};
    static const uint8_t durations[] = {1, 2, 3, 8, 30, 255};
    /* This binary links hardware.c and sfx.c, not sounds.c: lifecycle/SFX must
     * work without the tone-player array or any tone-player entry points. */
    for (uint8_t page = 0; page < 8; ++page) {
        zvb_config_dev_idx = page; n = 0;
        assert(sound_init_hardware() == ERR_SUCCESS);
        assert(zvb_config_dev_idx == page && master_volume == VOL_75 && held == 0);
        assert(last_mask == VOICEALL && played[0] == 0);
        assert(sound_deinit() == ERR_SUCCESS);
        assert(zvb_config_dev_idx == page && master_volume == VOL_0 && held == 1);
    }
    assert(resets == 8);
    unsigned sweeps = 0;
    for (unsigned a = 0; a < sizeof(hz)/sizeof(hz[0]); ++a)
        for (unsigned b = 0; b < sizeof(hz)/sizeof(hz[0]); ++b)
            for (unsigned d = 0; d < sizeof(durations); ++d) {
                sfx_seg effect[] = {SFX_SEG(WAV_NOISE, hz[a], hz[b], durations[d]), SFX_END};
                int64_t start = ((uint32_t)hz[a]*65536/44091)*65536LL;
                int64_t end = ((uint32_t)hz[b]*65536/44091)*65536LL;
                int64_t step = (end-start)/durations[d];
                n = 0; zvb_config_dev_idx = 5;
                sfx_init(VOICE3, VOL_100);
                assert(!sfx_active() && voice_volume == VOL_0);
                sfx_play(effect);
                for (unsigned i = 0; i <= durations[d]; ++i) sfx_tick();
                assert(n == durations[d] && !sfx_active() && voice_volume == VOL_0);
                assert(last_mask == VOICE3 && zvb_config_dev_idx == 5);
                assert(last_wave == (WAV_NOISE | DUTY_CYCLE_50_0));
                for (unsigned i = 0; i < n; ++i)
                    assert(played[i] == (uint16_t)((start+step*i)>>16));
                sweeps++;
            }
    static const sfx_seg low[] = {SFX_SEG(WAV_SQUARE, 440, 440, 2), SFX_END};
    static const sfx_seg high[] = {SFX_SEG(WAV_TRIANGLE, 880, 880, 1), SFX_END};
    static const sfx_seg empty[] = {SFX_END};
    static const sfx_seg multi[] = {SFX_SEG(WAV_SQUARE, 440, 440, 2),
        SFX_SEG(WAV_TRIANGLE, 880, 880, 1), SFX_END};
    static const sfx_seg noise[] = {SFX_SEG(WAV_NOISE, 100, 100, 1), SFX_END};
    for (uint8_t page = 0; page < 8; ++page) {
        zvb_config_dev_idx = page; n = 0;
        sfx_init(VOICE3, VOL_75);
        sfx_play(noise); sfx_tick(); sfx_tick();
        assert(!sfx_active() && volumes[3] == VOL_0);
        sfx_play(low);
        assert(sfx_active() && volumes[3] == VOL_0 && n == 1);
        /* Restoring before the first tick must not expose the stale noise. */
        sfx_restore(); assert(volumes[3] == VOL_0);
        sfx_tick();
        assert(volumes[3] == VOL_75 && unmuted_dividers[3] == SFX_DIV(440));
        assert(unmuted_waves[3] == (WAV_SQUARE | DUTY_CYCLE_50_0));
        assert(sfx_play_prio(high, 1));
        assert(volumes[3] == VOL_0);
        sfx_tick();
        assert(volumes[3] == VOL_75 && unmuted_dividers[3] == SFX_DIV(880));
        assert(unmuted_waves[3] == (WAV_TRIANGLE | DUTY_CYCLE_50_0));
        sfx_init(VOICE1, VOL_100);
        assert(!sfx_active() && volumes[3] == VOL_0 && volumes[1] == VOL_0);
        sfx_play(NULL); sfx_tick();
        assert(volumes[3] == VOL_0);
        sfx_play(low); sfx_tick();
        assert(volumes[1] == VOL_100 && volumes[3] == VOL_0);
        sfx_init(VOICE1, VOL_75);
        assert(!sfx_active() && volumes[1] == VOL_0);
        assert(zvb_config_dev_idx == page);
    }
    for (uint8_t page = 0; page < 8; ++page) {
        zvb_config_dev_idx = page; n = 0;
        sfx_init(VOICE1, VOL_75);
        assert(sfx_play_prio(low, 3));
        assert(!sfx_play_prio(high, 2));
        sfx_tick(); assert(played[0] == SFX_DIV(440));
        assert(!sfx_play_prio(NULL, 255) && !sfx_play_prio(empty, 255));
        assert(sfx_active());
        assert(sfx_play_prio(high, 3));
        sfx_tick(); assert(played[1] == SFX_DIV(880));
        voice_volume = VOL_0;
        sfx_restore(); assert(voice_volume == VOL_75);
        sfx_tick(); assert(!sfx_active() && voice_volume == VOL_0);
        sfx_restore(); assert(voice_volume == VOL_0);
        sfx_play(empty); sfx_tick();
        assert(!sfx_active() && voice_volume == VOL_0);
        sfx_play(low); sfx_play(NULL); sfx_tick();
        assert(!sfx_active() && voice_volume == VOL_0);
        sfx_play_prio(high, 255);
        sfx_play(low); /* unconditional play replaces even a higher-priority effect */
        sfx_tick(); assert(played[2] == SFX_DIV(440));
        sfx_init(VOICE1, VOL_75); assert(!sfx_active() && voice_volume == VOL_0);
        n = 0; sfx_play(multi);
        for (unsigned i = 0; i < 4; ++i) sfx_tick();
        assert(n == 3 && played[0] == SFX_DIV(440) && played[1] == SFX_DIV(440));
        assert(played[2] == SFX_DIV(880) && !sfx_active() && voice_volume == VOL_0);
        assert(zvb_config_dev_idx == page && last_mask == VOICE1);
    }
    printf("sfx: %u exact sweeps, priorities, empty effects, transitions, voice changes and paging passed\n", sweeps);
    return 0;
}
