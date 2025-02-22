#include <stdint.h>

#ifndef COMPRESS_RLE_H
#define COMPRESS_RLE_H

void decompress_rle(uint8_t *data, uint8_t size, uint8_t *buffer);

#endif