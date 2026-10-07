#pragma once
#include "zvb_hardware.h"
typedef uint8_t sound_voice_t;
typedef uint8_t sound_volume_t;
#define VOICE0 1
#define VOICE1 2
#define VOICE2 4
#define VOICE3 8
#define WAV_SQUARE 0
#define DUTY_CYCLE_50_0 128
#define VOL_0 0
#define VOL_25 1
#define VOL_50 2
#define VOL_75 3
#define VOL_100 4
#define SOUND_FREQ_TO_DIV(freq) (65536L*(freq)/44091)
extern uint8_t zvb_peri_sound_freq_low,zvb_peri_sound_freq_high,zvb_peri_sound_wave;
extern uint8_t zvb_peri_sound_select,zvb_peri_sound_volume,zvb_peri_sound_master_vol,zvb_peri_sound_hold;
