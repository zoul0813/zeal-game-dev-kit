#include <assert.h>
#include <stdio.h>
#include "zgdk/sound/sounds.h"

uint8_t zvb_config_dev_idx;
static uint16_t dividers[MAX_VOICES], waveforms[MAX_VOICES];
static uint8_t held;
static sound_volume_t volume;
static unsigned calls, resets;

/* Like the real SDK, every hardware operation leaves sound mapped. */
void zvb_sound_initialize(uint8_t reset) {
    zvb_map_peripheral(ZVB_PERI_SOUND_IDX);
    assert(reset == 1);
    ++resets;
}

void zvb_sound_set_voices(sound_voice_t voices, uint16_t divider, sound_waveform_t waveform) {
    zvb_map_peripheral(ZVB_PERI_SOUND_IDX);
    ++calls;
    for (unsigned i = 0; i < MAX_VOICES; ++i) {
        if (voices & (1U << i)) {
            dividers[i] = divider;
            waveforms[i] = waveform;
        }
    }
}

void zvb_sound_set_hold(sound_voice_t voices, uint8_t hold) {
    zvb_map_peripheral(ZVB_PERI_SOUND_IDX);
    if (hold) held |= voices;
    else held &= ~voices;
}

void zvb_sound_set_volume(sound_volume_t value) {
    zvb_map_peripheral(ZVB_PERI_SOUND_IDX);
    volume = value;
}

int main(void) {
    for (uint8_t page = 0; page < 8; ++page) {
        zvb_config_dev_idx = page;
        assert(sound_init() == ERR_SUCCESS);
        assert(zvb_config_dev_idx == page && volume == VOL_75 && held == 0);
        for (uint8_t i = 0; i < MAX_VOICES; ++i) {
            Sound *sound = sound_get(i);
            assert(sound && sound->voice == (1U << i));
            assert(sound->remaining == 0 && sound->duration == 0 && sound->freq == 440);
            assert(sound_set(i, WAV_SQUARE) == sound);
            assert(zvb_config_dev_idx == page);
            assert(sound_play(i, 440, 2) == sound);
            assert(zvb_config_dev_idx == page);
            assert(dividers[i] == SOUND_FREQ_TO_DIV(440));
            assert(waveforms[i] == (WAV_SQUARE | DUTY_CYCLE_50_0));
            sound_stop(sound);
            assert(sound->remaining == 0 && dividers[i] == 0);
            assert(zvb_config_dev_idx == page);
        }
        sound_stop_all();
        assert(zvb_config_dev_idx == page);
        assert(sound_deinit() == ERR_SUCCESS);
        assert(zvb_config_dev_idx == page && held == VOICEALL && volume == VOL_0);
    }
    assert(resets == 8);

    sound_init();
    zvb_config_dev_idx = 5;
    for (uint16_t duty = DUTY_CYCLE_12_5; duty <= DUTY_CYCLE_87_5; duty += 0x20) {
        assert(sound_set(0, WAV_SQUARE | duty));
        assert(sound_play(0, 220, 3));
        assert(waveforms[0] == (WAV_SQUARE | duty));
        assert(dividers[0] == SOUND_FREQ_TO_DIV(220));
        assert(zvb_config_dev_idx == 5);
    }
    for (uint16_t wave = WAV_SQUARE; wave <= WAV_NOISE; ++wave) {
        sound_set(0, wave);
        sound_play(0, 440, 3);
        assert(waveforms[0] == (wave | DUTY_CYCLE_50_0));
        assert(sound_get(0)->waveform == wave);
    }

    unsigned before = calls;
    assert(sound_play(MAX_VOICES, 440, 2) == NULL);
    assert(sound_play(255, 440, 2) == NULL);
    assert(sound_set(MAX_VOICES, WAV_NOISE) == NULL);
    assert(sound_set(255, WAV_NOISE) == NULL);
    assert(sound_get(MAX_VOICES) == NULL && sound_get(255) == NULL);
    assert(calls == before && zvb_config_dev_idx == 5);

    /* Duration is measured in loop calls. Zero duration remains continuous. */
    for (uint8_t i = 0; i < MAX_VOICES; ++i) sound_play(i, 440, i);
    for (uint8_t tick = 1; tick <= 3; ++tick) {
        sound_loop();
        assert(zvb_config_dev_idx == 5);
        for (uint8_t i = 0; i < MAX_VOICES; ++i) {
            assert(sound_get(i)->remaining == (i > tick ? i - tick : 0));
            assert(dividers[i] == (i && i <= tick ? 0 : SOUND_FREQ_TO_DIV(440)));
            assert(sound_get(i)->duration == i);
        }
    }
    before = calls;
    sound_loop();
    assert(calls == before && zvb_config_dev_idx == 5);
    sound_stop_all();
    for (uint8_t i = 0; i < MAX_VOICES; ++i) assert(dividers[i] == 0);
    puts("sound regression checks passed");
    return 0;
}
