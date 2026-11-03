#include <assert.h>
#include <stdint.h>
#include "zgdk/anims.h"

int main(void) {
    uint8_t frames[255];
    const uint8_t counts[] = {1, 2, 4, 255};
    const uint8_t rates[] = {1, 6, 255};
    anim_t a = {0};
    unsigned i, c, r, loop;
    for (i = 0; i < 255; ++i) frames[i] = (i * 31 + 128) & 255;
    assert(anim_tile(&a) == 0 && anim_tick(&a) == 0);
    for (c = 0; c < sizeof(counts); ++c) {
        for (r = 0; r < sizeof(rates); ++r) {
            for (loop = 0; loop < 2; ++loop) {
                anim_def def = { frames, counts[c], rates[r], loop ? ANIM_LOOP : 0 };
                uint32_t ticks, length = (uint32_t)def.count * def.rate;
                anim_restart(&a, &def);
                assert(a.index == 0 && a.timer == 0 && !a.done);
                assert(anim_tile(&a) == frames[0]);
                for (ticks = 1; ticks <= length * 2 + 3; ++ticks) {
                    unsigned index = loop ? (ticks / def.rate) % def.count :
                        (ticks >= length ? def.count - 1 : ticks / def.rate);
                    unsigned rendered = loop ? ((ticks - 1) / def.rate) % def.count :
                        (ticks > length ? def.count - 1 : (ticks - 1) / def.rate);
                    uint8_t timer = !loop && ticks >= length ? 0 : ticks % def.rate;
                    assert(anim_tick(&a) == frames[rendered]);
                    assert(a.index == index && a.timer == timer);
                    assert(a.done == (!loop && ticks >= length));
                    anim_play(&a, &def); /* Repeated selection preserves every field. */
                    assert(a.index == index && a.timer == timer);
                    assert(a.done == (!loop && ticks >= length));
                    assert(anim_tile(&a) == frames[index]);
                }
                anim_play(&a, NULL);
                assert(!a.def && !a.index && !a.timer && !a.done);
                assert(anim_tile(&a) == 0 && anim_tick(&a) == 0);
            }
        }
    }
    {
        const anim_def first = {frames, 2, 1, ANIM_LOOP};
        const anim_def second = {frames + 2, 2, 2, 0};
        anim_play(&a, &first);
        anim_tick(&a);
        assert(a.index == 1);
        anim_restart(&a, &first);
        assert(!a.index && !a.timer && !a.done);
        anim_tick(&a);
        anim_play(&a, &second);
        assert(a.def == &second && !a.index && !a.timer && !a.done);
    }
    return 0;
}
