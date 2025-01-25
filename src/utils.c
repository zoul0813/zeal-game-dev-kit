#include <stdio.h>

#include "utils.h"
#include "ascii.h"
#include <string.h>

void print_string(gfx_context* ctx, const char* str, uint8_t x, uint8_t y)
{
    return nprint_string(ctx, str, strlen(str), x, y);
}

void nprint_string(gfx_context* ctx, const char* str, uint8_t len, uint8_t x, uint8_t y)
{
    uint8_t line[40];
    for(uint8_t i = 0; i < len; i++) {
        line[i] = ascii_to(str[i]);
    }
    gfx_tilemap_load(ctx, line, len, 1, x, y);
}

char rand8_quick(void) __naked
{
    __asm__(
    "ld a, r\n"
    "ret\n"
    );
}

static uint16_t rseed = 1;
void rand8_seed(uint16_t seed) {
    if(seed != 0) {
        rseed = seed;
    }
}

char rand8(void) __naked
{
    // credit: https://spectrumcomputing.co.uk/forums/viewtopic.php?t=4571
    // __asm
    // ;This code snippet is 9 bytes and 43cc
    // ;Inputs:
    // ;   HL is the input seed and must be non-zero
    // ;Outputs:
    // ;   A is the 8-bit pseudo-random number
    // ;   HL is the new seed value (will be non-zero)

    // ;-------------------------------------------------------------------------------
    // ;Technical details:
    // ;   The concept behind this routine is to combine an LFSR (poor RNG) with a
    // ; counter. The counter improves the RNG quality, while also extending the period
    // ; length.
    // ;   For this routine, I took advantage of the Z80's built-in counter, the `r`
    // ; register. This means that we don't need to store the counter anywhere, and it
    // ; is pretty fast to access!
    // ;   Some caveats:
    // ;     * r is a 7-bit counter
    // ;     * r will increment some number of times between runs of the RNG. In most
    // ;       cases, this will be constant, but if it increments an even number each
    // ;       time, then the bottom bit is always the same, weakening the effect of
    // ;       the counter. In the worst case, it increments a multiple of 128 times,
    // ;       effectively making your RNG just as good/bad as the LFSR. Ideally, you
    // ;       want `r` to increment an odd number of times between runs.
    // ;     * In the best case, the bottom 7 bits have 50/50 chance of being 0 or 1.
    // ;       The top bit is 1 with probability 1/2 + 1/(2^17-2) ~ .5000076295
    // ;     * In the event that your main loop waits for user input between calls,
    // ;       then congatulations, you might have a True RNG :)
    // ;-------------------------------------------------------------------------------

    __asm__(
    "ld hl, (_rseed)\n"
    "add hl,hl\n"
    "sbc a,a\n"
    "and #0x2D\n"
    "xor l\n"
    "ld l,a\n"
    "ld a,r\n"
    "add a,h\n"
    "ld (_rseed), hl\n"
    "ret\n"
    );
}
