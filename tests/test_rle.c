#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "zgdk/compress.h"

static uint8_t encoded[65535];
static uint8_t output[65536];
static uint8_t expected[65536];

static int check(uint16_t size, int wrapper)
{
    uint16_t i = 0;
    uint16_t count = 0;
    while (size - i >= 2) {
        /* An odd input length ends with a two-byte literal packet. */
        if (size - i == 3) {
            encoded[i++] = 1;
            encoded[i++] = 42;
            encoded[i++] = 43;
            expected[count++] = 42;
            expected[count++] = 43;
        } else {
            encoded[i++] = 0x80;
            encoded[i++] = (uint8_t)count;
            expected[count] = (uint8_t)count;
            count++;
        }
    }
    memset(output, 0xa5, sizeof(output));
    if (wrapper)
        decompress(COMPRESSION_RLE, encoded, size, output);
    else
        decompress_rle(encoded, size, output);
    if (memcmp(output, expected, count) || output[count] != 0xa5) {
        fprintf(stderr, "%s failed for %u compressed bytes\n",
                wrapper ? "decompress" : "decompress_rle", size);
        return 1;
    }
    return 0;
}

int main(void)
{
    const uint16_t sizes[] = {286, 262, 254, 255, 256, 65534, 65535, 0, 2};
    unsigned int i;
    int wrapper;
    for (wrapper = 0; wrapper < 2; wrapper++)
        for (i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++)
            if (check(sizes[i], wrapper))
                return 1;
    /* Cover maximum repeat and literal packet lengths. */
    encoded[0] = 0xff;
    encoded[1] = 42;
    encoded[2] = 127;
    for (i = 0; i < 128; i++)
        encoded[i + 3] = (uint8_t)i;
    for (wrapper = 0; wrapper < 2; wrapper++) {
        memset(output, 0xa5, sizeof(output));
        if (wrapper)
            decompress(COMPRESSION_RLE, encoded, 131, output);
        else
            decompress_rle(encoded, 131, output);
        for (i = 0; i < 128; i++)
            if (output[i] != 42 || output[i + 128] != (uint8_t)i)
                return 1;
        if (output[256] != 0xa5)
            return 1;
    }
    puts("RLE regression checks passed");
    return 0;
}
