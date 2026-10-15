#include <zvb_sound.h>
#include "zgdk/sound/tracker.h"
#include "private.h"

const __sfr __banked __at(0xF0) mmu_page0_ro;

uint8_t zmt_voice_volumes[NUM_VOICES];
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
