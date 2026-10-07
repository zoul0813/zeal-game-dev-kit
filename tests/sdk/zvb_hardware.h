#pragma once
#include <stdint.h>
#define __sfr uint8_t
#define __banked
#define __at(a)
extern uint8_t zvb_config_dev_idx;
#define ZVB_PERI_SOUND_IDX 2
void zvb_map_peripheral(uint8_t);
