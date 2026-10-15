#pragma once
#include <stdint.h>
#define __sfr extern uint8_t
#define __banked
#define __at(x)
extern uint8_t zvb_peri_sound_freq_low, zvb_peri_sound_freq_high;
extern uint8_t zvb_peri_sound_wave, zvb_peri_sound_select, zvb_peri_sound_volume;
extern uint8_t zvb_peri_sound_master_vol, zvb_peri_sound_hold, zvb_config_dev_idx;
extern uint8_t zvb_peri_sound_left_channel, zvb_peri_sound_right_channel;
#define zvb_map_peripheral(x) (zvb_config_dev_idx=(x))
#define ZVB_PERI_SOUND_IDX 3
