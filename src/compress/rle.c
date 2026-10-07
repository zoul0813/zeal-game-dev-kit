#include "zgdk/compress/rle.h"

bool decompress_rle_checked(const uint8_t *data, uint16_t size,
                            uint8_t *buffer, uint16_t capacity)
{
    uint16_t i = 0, expanded = 0;
    if (data == NULL || buffer == NULL || size == 0)
        return false;

    /* Validate the whole stream before modifying the destination. */
    while (i < size) {
        uint8_t header = data[i++];
        uint8_t length = (header & 0x7f) + 1;
        uint8_t consumed = (header & 0x80) ? 1 : length;
        if (consumed > size - i || length > capacity - expanded)
            return false;
        i += consumed;
        expanded += length;
    }

    i = 0;
    uint16_t j = 0;
    while (i < size) {
        uint8_t header = data[i++];
        uint8_t length = (header & 0x7f) + 1;
        if (header & 0x80) {
            uint8_t value = data[i++];
            while (length--)
                buffer[j++] = value;
        } else {
            while (length--)
                buffer[j++] = data[i++];
        }
    }
    return true;
}

void decompress_rle(uint8_t *data, uint8_t size, uint8_t *buffer)
{
    /* The legacy API still requires callers to allocate sufficient output. */
    decompress_rle_checked(data, size, buffer, UINT16_MAX);
}
