#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifndef COMPRESS_RLE_H
#define COMPRESS_RLE_H

/* Reject empty/truncated streams or insufficient capacity without writing output. */
bool decompress_rle_checked(const uint8_t *data, uint16_t size,
                            uint8_t *buffer, uint16_t capacity);

/* Malformed input is ignored; output capacity remains the caller's responsibility. */
void decompress_rle(uint8_t *data, uint8_t size, uint8_t *buffer);

#endif