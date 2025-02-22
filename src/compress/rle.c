#include "zgdk/compress.h"

void decompress_rle(uint8_t *data, uint8_t size, uint8_t *buffer) {
    uint8_t length = 0;
    uint16_t i = 0; // data index
    uint16_t j = 0; // buffer index
    uint8_t k;
    uint16_t length_of_data = size - 1;

    while(i < length_of_data) {
        length = data[i];
        i++;
        if(length >= 0x80) {
            length = (length - 0x80) + 1;
            uint8_t value = data[i];
            while(length--) {
                buffer[j++] = value;
            }
            i++;
        } else {
            length++;
            // memcpy(&buffer[j], &data[i], length)
            for(k = 0; k < length; k++) {
                buffer[j + k] = data[i + k];
            }
            i += length;
            j += length;
        }
    }
}