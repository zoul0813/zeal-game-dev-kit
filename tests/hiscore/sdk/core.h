#pragma once
#include <stdint.h>
#include <string.h>
#define mem_cpy memcpy
#define mem_set memset
#define mem_cmp memcmp
void itoa_pad(uint16_t, char *, uint16_t, char, char, uint8_t);
void msleep(uint16_t);
