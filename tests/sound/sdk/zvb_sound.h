#pragma once
#include "zvb_hardware.h"
#define SOUND_FREQ_TO_DIV(freq) (65536UL * (freq) / 44091)
typedef enum { VOICE0=1, VOICE1=2, VOICE2=4, VOICE3=8 } sound_voice_t;
typedef enum { WAV_SQUARE, WAV_TRIANGLE, WAV_SAWTOOTH, WAV_NOISE } sound_waveform_t;
typedef enum { VOL_0=128, VOL_25=0, VOL_50=1, VOL_75=2, VOL_100=3 } sound_volume_t;
typedef enum {
    DUTY_CYCLE_12_5=0x20, DUTY_CYCLE_25_0=0x40, DUTY_CYCLE_37_5=0x60,
    DUTY_CYCLE_50_0=0x80, DUTY_CYCLE_62_5=0xa0, DUTY_CYCLE_75_0=0xc0,
    DUTY_CYCLE_87_5=0xe0
} sound_duty_cycle_t;
void zvb_sound_initialize(uint8_t);
void zvb_sound_set_voices(sound_voice_t, uint16_t, sound_waveform_t);
void zvb_sound_set_hold(sound_voice_t, uint8_t);
void zvb_sound_set_volume(sound_volume_t);

void zvb_sound_set_voices_vol(sound_voice_t, sound_volume_t);
