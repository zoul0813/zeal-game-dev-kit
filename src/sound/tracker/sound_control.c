#include <zvb_sound.h>
#include "zgdk/sound/tracker.h"
#include "private.h"

const __sfr __banked __at(0xF0) mmu_page0_ro;

/* Internal - set divider */
void SOUND_DIV(uint16_t div)
{
    zvb_peri_sound_freq_low  = div & 0xff;
    zvb_peri_sound_freq_high = (div >> 8) & 0xff;
}
/* Internal - set waveform */
void SOUND_WAVE(uint8_t waveform)
{
    zvb_peri_sound_wave = (waveform & 03) | DUTY_CYCLE_50_0;
}
/* Internal - set voice selection */
void SOUND_SELECT(uint8_t voices)
{
    zvb_peri_sound_select = voices;
}
/* Internal - sound off */
void SOUND_OFF(void)
{
    SOUND_SELECT(VOICEALL);
    zvb_peri_sound_volume = VOL_0;
    zvb_peri_sound_freq_low  = 0x00;
    zvb_peri_sound_freq_high = 0x00;
}
/* Internal - set volume */
void SOUND_VOL(uint8_t vol)
{
    uint8_t level = vol & 0x03;
    uint8_t disabled = vol & 0x80;

    SOUND_SELECT(VOICEALL);
    zvb_peri_sound_volume = vol;
    zvb_peri_sound_master_vol = disabled | (disabled >> 1) | level | (level << 2);
}
/* Internal - set volume, hold voices, sound off */
void SOUND_RESET(uint8_t vol)
{
    SOUND_SELECT(VOICEALL);
    SOUND_WAVE(WAV_SQUARE);
    zvb_peri_sound_left_channel = VOICEALL;
    zvb_peri_sound_right_channel = VOICEALL;
    SOUND_VOL(vol);
    zvb_peri_sound_hold &= ~VOICEALL;
    SOUND_OFF();
}

static uint8_t zmt_voice_volumes[NUM_VOICES];
static uint8_t active_voices;

/* A zero phase increment cannot silence noise: use the voice mute bit. */
void zmt_voice_active(sound_voice_t voice, uint8_t active)
{
    SOUND_SELECT(voice);
    if (active) {
        active_voices |= voice;
        for (uint8_t i = 0; i < NUM_VOICES; i++) {
            if (voice == (1U << i)) {
                zvb_peri_sound_volume = zmt_voice_volumes[i];
                break;
            }
        }
    } else {
        active_voices &= ~voice;
        zvb_peri_sound_volume = VOL_0;
    }
}

void zmt_voice_volume(sound_voice_t voice, sound_volume_t volume)
{
    uint8_t selected = zvb_peri_sound_select;
    for (uint8_t i = 0; i < NUM_VOICES; i++) {
        if (voice == (1U << i)) {
            zmt_voice_volumes[i] = volume;
            SOUND_SELECT(voice);
            zvb_peri_sound_volume = (active_voices & voice) ? volume : VOL_0;
            break;
        }
    }
    SOUND_SELECT(selected);
}

void zmt_sound_off(void)
{
    uint8_t backup_page = zvb_config_dev_idx;
    zvb_map_peripheral(ZVB_PERI_SOUND_IDX);
    SOUND_OFF();
    active_voices = 0;
    zvb_map_peripheral(backup_page);
}

/* reset the sound system, setting volume */
void zmt_reset(sound_volume_t vol)
{
    uint8_t backup_page = zvb_config_dev_idx;
    zvb_map_peripheral(ZVB_PERI_SOUND_IDX);
    SOUND_RESET(vol);
    active_voices = 0;
    for (uint8_t i = 0; i < NUM_VOICES; i++)
        zmt_voice_volumes[i] = vol;
    zvb_map_peripheral(backup_page);
}
