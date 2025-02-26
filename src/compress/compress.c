#include "zgdk/compress.h"

void decompress(CompressionType type, uint8_t *data, uint8_t size, uint8_t *buffer) {
    switch(type) {
        case COMPRESSION_RLE:
            return decompress_rle(data, size, buffer);
    }
}