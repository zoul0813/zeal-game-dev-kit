#pragma once
#include "zvb_hardware.h"
#define SOUND_FREQ_TO_DIV(freq) (65536UL * (freq) / 44091)
typedef enum { VOICE0=1, VOICE1=2, VOICE2=4, VOICE3=8, SAMPTAB=128 } sound_voice_t;
typedef enum { WAV_SQUARE, WAV_TRIANGLE, WAV_SAWTOOTH, WAV_NOISE } sound_waveform_t;
typedef enum { VOL_0=128, VOL_25=0, VOL_50=1, VOL_75=2, VOL_100=3 } sound_volume_t;
#define DUTY_CYCLE_50_0 0x80
