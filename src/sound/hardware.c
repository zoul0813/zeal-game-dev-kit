#include <stdint.h>
#include <zvb_hardware.h>
#include <zvb_sound.h>
#include "zgdk/sound/sounds.h"

/* Kept separate from sounds.c so tracker/SFX-only programs do not link its
 * Sound array or tone-player helpers. Both paths preserve peripheral mapping. */
zos_err_t sound_init_hardware(void) {
    uint8_t page = zvb_config_dev_idx;
    zvb_sound_initialize(1);
    zvb_sound_set_voices(VOICEALL, 0, WAV_SQUARE | DUTY_CYCLE_50_0);
    zvb_sound_set_hold(VOICEALL, 0);
    zvb_sound_set_volume(VOL_75);
    zvb_map_peripheral(page);
    return ERR_SUCCESS;
}

zos_err_t sound_deinit(void) {
    uint8_t page = zvb_config_dev_idx;
    zvb_sound_set_voices(VOICEALL, 0, WAV_SQUARE | DUTY_CYCLE_50_0);
    zvb_sound_set_hold(VOICEALL, 1);
    zvb_sound_set_volume(VOL_0);
    zvb_map_peripheral(page);
    return ERR_SUCCESS;
}
