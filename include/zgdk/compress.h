#include "zgdk/compress/rle.h"

typedef enum {
    COMPRESSION_RLE = 0,
} CompressionType;

void decompress(CompressionType type, uint8_t *data, uint8_t size, uint8_t *buffer);