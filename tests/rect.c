#include <assert.h>
#include <stdio.h>
#include "zgdk/rect.h"

static void expect(Rect a, Rect b, uint8_t overlap) {
    assert(rect_collide(&a, &b) == overlap);
    assert(rect_collide(&b, &a) == overlap);
}

static uint8_t reference(Rect a, Rect b) {
    return a.w && a.h && b.w && b.h
        && (int32_t)a.x - a.w < b.x && (int32_t)b.x - b.w < a.x
        && (int32_t)a.y - a.h < b.y && (int32_t)b.y - b.h < a.y;
}

static uint32_t seed = 1;
static uint16_t next_value(void) {
    seed = seed * 1664525UL + 1013904223UL;
    return seed >> 16;
}

int main(void) {
    Rect a = {.x=32, .y=32, .w=16, .h=16};
    expect(a, a, 1);
    expect(a, (Rect){.x=40, .y=32, .w=16, .h=16}, 1);
    expect(a, (Rect){.x=32, .y=40, .w=16, .h=16}, 1);
    expect(a, (Rect){.x=40, .y=40, .w=16, .h=16}, 1);
    expect(a, (Rect){.x=24, .y=24, .w=4, .h=4}, 1);
    expect(a, (Rect){.x=48, .y=32, .w=16, .h=16}, 0);
    expect(a, (Rect){.x=32, .y=48, .w=16, .h=16}, 0);
    expect(a, (Rect){.x=48, .y=48, .w=16, .h=16}, 0);
    expect(a, (Rect){.x=60, .y=60, .w=16, .h=16}, 0);
    expect(a, (Rect){.x=32, .y=32, .w=0, .h=16}, 0);
    expect(a, (Rect){.x=32, .y=32, .w=16, .h=0}, 0);
    expect((Rect){.x=30, .y=30, .w=5, .h=5},
           (Rect){.x=20, .y=20, .w=20, .h=20}, 0);
    expect((Rect){.x=0, .y=0, .w=16, .h=16},
           (Rect){.x=1, .y=1, .w=16, .h=16}, 1);
    expect((Rect){.x=65535, .y=65535, .w=65535, .h=65535},
           (Rect){.x=1, .y=1, .w=1, .h=1}, 1);
    expect((Rect){.x=65535, .y=65535, .w=1, .h=1},
           (Rect){.x=0, .y=0, .w=1, .h=1}, 0);
    expect((Rect){.x=32768, .y=32768, .w=1, .h=1},
           (Rect){.x=32767, .y=32767, .w=1, .h=1}, 0);

    /* Compare with a wider signed AABB oracle over the full uint16_t range. */
    for (unsigned i = 0; i < 10000; ++i) {
        Rect b;
        a.x=next_value(); a.y=next_value(); a.w=next_value(); a.h=next_value();
        b.x=next_value(); b.y=next_value(); b.w=next_value(); b.h=next_value();
        expect(a, b, reference(a, b));
    }
    puts("rectangle regression checks passed");
    return 0;
}
