#include <stdint.h>
#include <stddef.h>
#include "zgdk/types.h"
#include "zgdk/physics/tileflags.h"
#include "zgdk/physics/tilemap.h"
#include "tileflags_cache.h"

/*
 * Edge probes walk the tiles an edge spans (2 tiles for a 16px edge, 3 for 32px) and read
 * the cached tile buffer directly. Off-map tiles report the tileflags_set_outside() value.
 * tileflags_ready() is checked once per body_step().
 * On SDCC they are hand-written Z80 (the hottest body code); the C versions are the reference
 * (host tests, or build with -DBODY_PROBE_C).
 */
#if defined(__SDCC) && !defined(BODY_PROBE_C)
_Static_assert(sizeof(tileflags_rows[0]) == 2, "probe asm requires two-byte row pointers");
static uint8_t probe_row(int16_t py, int16_t left, int16_t right) __naked {
    (void)py; (void)left; (void)right;
    __asm
    ; hl = py, de = left, right on the stack (callee pops)
    pop     iy
    pop     bc
    push    iy
    ; the fixed coordinate must be on the map
    bit     7, h
    jp      NZ, 00190$
    srl     h
    rr      l
    srl     h
    rr      l
    srl     h
    rr      l
    srl     h
    rr      l
    ld      a, h
    or      a, a
    jp      NZ, 00190$
    ld      a, l
    ld      hl, #__tf_height
    cp      a, (hl)
    jp      NC, 00190$
    push    af
    ; first tile in E, flags in D (outside when the span starts off-map)
    bit     7, d
    jr      Z, 00110$
    ld      a, (__tf_outside)
    ld      d, a
    ld      e, #0
    jr      00112$
00110$:
    srl     d
    rr      e
    srl     d
    rr      e
    srl     d
    rr      e
    srl     d
    rr      e
    ld      a, d
    ld      d, #0
    or      a, a
    jr      Z, 00112$
    ld      e, #0xFF
00112$:
    ; last tile in C, clamped to __tf_width - 1 (adds outside)
    bit     7, b
    jr      NZ, 00185$
    srl     b
    rr      c
    srl     b
    rr      c
    srl     b
    rr      c
    srl     b
    rr      c
    ld      hl, #__tf_width
    ld      a, b
    or      a, a
    jr      NZ, 00114$
    ld      a, c
    cp      a, (hl)
    jr      C, 00116$
00114$:
    ld      a, (__tf_outside)
    or      a, d
    ld      d, a
    ld      a, (hl)
    or      a, a
    jr      Z, 00185$
    dec     a
    ld      c, a
00116$:
    ; C = tile count, none when first > last
    ld      a, c
    sub     a, e
    jr      C, 00185$
    inc     a
    ld      c, a
    ; hl = rows[ty] + first
    pop     af
    push    de
    ld      l, a
    ld      h, #0
    add     hl, hl
    ld      de, #_tileflags_rows
    add     hl, de
    ld      e, (hl)
    inc     hl
    ld      d, (hl)
    ex      de, hl
    pop     de
    ld      a, e
    add     a, l
    ld      l, a
    adc     a, h
    sub     a, l
    ld      h, a
    ld      b, d
    ld      de, (__tf_flags)
00120$:
    ld      a, (hl)
    inc     hl
    push    hl
    add     a, e
    ld      l, a
    adc     a, d
    sub     a, l
    ld      h, a
    ld      a, b
    or      a, (hl)
    ld      b, a
    pop     hl
    dec     c
    jr      NZ, 00120$
    ld      a, b
    ret
00185$:
    pop     af
    ld      a, d
    ret
00190$:
    ld      a, (__tf_outside)
    ret
    __endasm;
}

static uint8_t probe_column(int16_t px, int16_t top, int16_t bottom) __naked {
    (void)px; (void)top; (void)bottom;
    __asm
    ; hl = px, de = top, bottom on the stack (callee pops)
    pop     iy
    pop     bc
    push    iy
    ; the fixed coordinate must be on the map
    bit     7, h
    jp      NZ, 00190$
    srl     h
    rr      l
    srl     h
    rr      l
    srl     h
    rr      l
    srl     h
    rr      l
    ld      a, h
    or      a, a
    jp      NZ, 00190$
    ld      a, l
    ld      hl, #__tf_width
    cp      a, (hl)
    jp      NC, 00190$
    push    af
    ; first tile in E, flags in D (outside when the span starts off-map)
    bit     7, d
    jr      Z, 00110$
    ld      a, (__tf_outside)
    ld      d, a
    ld      e, #0
    jr      00112$
00110$:
    srl     d
    rr      e
    srl     d
    rr      e
    srl     d
    rr      e
    srl     d
    rr      e
    ld      a, d
    ld      d, #0
    or      a, a
    jr      Z, 00112$
    ld      e, #0xFF
00112$:
    ; last tile in C, clamped to __tf_height - 1 (adds outside)
    bit     7, b
    jr      NZ, 00185$
    srl     b
    rr      c
    srl     b
    rr      c
    srl     b
    rr      c
    srl     b
    rr      c
    ld      hl, #__tf_height
    ld      a, b
    or      a, a
    jr      NZ, 00114$
    ld      a, c
    cp      a, (hl)
    jr      C, 00116$
00114$:
    ld      a, (__tf_outside)
    or      a, d
    ld      d, a
    ld      a, (hl)
    or      a, a
    jr      Z, 00185$
    dec     a
    ld      c, a
00116$:
    ; C = tile count, none when first > last
    ld      a, c
    sub     a, e
    jr      C, 00185$
    inc     a
    ld      c, a
    ; hl = rows[first] + tx
    pop     af
    push    de
    push    bc
    ld      c, a
    ld      l, e
    ld      h, #0
    add     hl, hl
    ld      de, #_tileflags_rows
    add     hl, de
    ld      e, (hl)
    inc     hl
    ld      d, (hl)
    ex      de, hl
    ld      b, #0
    add     hl, bc
    pop     bc
    pop     de
    ld      b, d
    ld      de, (__tf_flags)
00140$:
    push    hl
    ld      a, (hl)
    add     a, e
    ld      l, a
    adc     a, d
    sub     a, l
    ld      h, a
    ld      a, b
    or      a, (hl)
    ld      b, a
    pop     hl
    ld      a, (__tf_width)
    add     a, l
    ld      l, a
    adc     a, h
    sub     a, l
    ld      h, a
    dec     c
    jr      NZ, 00140$
    ld      a, b
    ret
00185$:
    pop     af
    ld      a, d
    ret
00190$:
    ld      a, (__tf_outside)
    ret
    __endasm;
}
#else
static uint8_t probe_row(int16_t py, int16_t left, int16_t right) {
    uint8_t flags = 0;
    int16_t tx, tx1;
    const uint8_t *row;
    int16_t ty = py >> 4;
    if (py < 0 || ty >= _tf_height) return _tf_outside;
    row = tileflags_rows[(uint8_t)ty];
    tx = left >> 4;
    tx1 = right >> 4;
    if (left < 0) { flags = _tf_outside; tx = 0; }
    if (tx1 >= _tf_width) { flags |= _tf_outside; tx1 = _tf_width - 1; }
    for (; tx <= tx1; tx++) flags |= _tf_flags[row[(uint8_t)tx]];
    return flags;
}

static uint8_t probe_column(int16_t px, int16_t top, int16_t bottom) {
    uint8_t flags = 0;
    int16_t ty, ty1;
    int16_t tx = px >> 4;
    if (px < 0 || tx >= _tf_width) return _tf_outside;
    ty = top >> 4;
    ty1 = bottom >> 4;
    if (top < 0) { flags = _tf_outside; ty = 0; }
    if (ty1 >= _tf_height) { flags |= _tf_outside; ty1 = _tf_height - 1; }
    for (; ty <= ty1; ty++) flags |= _tf_flags[tileflags_rows[(uint8_t)ty][(uint8_t)tx]];
    return flags;
}
#endif

/* pixel edges of the body being stepped, filled by edges(): globals are much cheaper than SDCC stack locals */
static int16_t E_LEFT, E_RIGHT, E_TOP, E_BOTTOM;

#if defined(__SDCC) && !defined(BODY_PROBE_C)
_Static_assert(offsetof(body_t, x) == 0 && offsetof(body_t, y) == 2, "edges asm offsets");
_Static_assert(offsetof(body_t, w) == 8 && offsetof(body_t, h) == 9, "edges asm offsets");

static void edges(const body_t *body) __naked {
    (void)body;
    __asm
    ; hl = body
    ld      e, (hl)
    inc     hl
    ld      d, (hl)
    inc     hl
    sra     d
    rr      e
    sra     d
    rr      e
    sra     d
    rr      e
    sra     d
    rr      e
    ld      (_E_LEFT), de
    ld      c, (hl)
    inc     hl
    ld      b, (hl)
    sra     b
    rr      c
    sra     b
    rr      c
    sra     b
    rr      c
    sra     b
    rr      c
    ld      (_E_TOP), bc
    push    bc
    ld      bc, #5
    add     hl, bc
    ld      c, (hl)
    ld      b, #0
    dec     bc
    ex      de, hl
    add     hl, bc
    ld      (_E_RIGHT), hl
    ex      de, hl
    inc     hl
    ld      c, (hl)
    ld      b, #0
    dec     bc
    pop     hl
    add     hl, bc
    ld      (_E_BOTTOM), hl
    ret
    __endasm;
}
#else
static void edges(const body_t *body) {
    E_LEFT = PHYSICS_FP_INT(body->x);
    E_TOP = PHYSICS_FP_INT(body->y);
    E_RIGHT = E_LEFT + body->w - 1;
    E_BOTTOM = E_TOP + body->h - 1;
}
#endif

static void move_x(body_t *body) {
    if (body->vx == 0) return;
    body->x += body->vx;
    edges(body);
    if (body->vx > 0) {
        if (probe_column(E_RIGHT, E_TOP, E_BOTTOM) & TF_SOLID) {
            body->x = PHYSICS_FP((E_RIGHT & ~15) - body->w);
            body->vx = 0;
            body->state |= BODY_HIT_WALL;
        }
    } else {
        if (probe_column(E_LEFT, E_TOP, E_BOTTOM) & TF_SOLID) {
            body->x = PHYSICS_FP((E_LEFT & ~15) + 16);
            body->vx = 0;
            body->state |= BODY_HIT_WALL;
        }
    }
}

static void move_y(body_t *body) {
    int16_t old_bottom;
    uint8_t flags;
    if (body->vy == 0) return;
    edges(body);
    old_bottom = E_BOTTOM;
    body->y += body->vy;
    edges(body);
    if (body->vy > 0) {
        flags = probe_row(E_BOTTOM, E_LEFT, E_RIGHT);
        // one-way tiles only catch a body whose feet were above the tile last frame
        // one-way tiles don't catch a climbing body, so it can climb down through a ladder top
        if ((flags & TF_SOLID) ||
            ((flags & TF_ONEWAY) && !(body->state & BODY_PASS_ONEWAY) && (old_bottom >> 4) < (E_BOTTOM >> 4))) {
            body->y = PHYSICS_FP((E_BOTTOM & ~15) - body->h);
            body->vy = 0;
            body->state |= BODY_ON_GROUND;
        }
    } else {
        if (probe_row(E_TOP, E_LEFT, E_RIGHT) & TF_SOLID) {
            body->y = PHYSICS_FP((E_TOP & ~15) + 16);
            body->vy = 0;
            body->state |= BODY_HIT_CEIL;
        }
    }
}

/* a body resting exactly on a tile top doesn't sink into it, so check the row under the feet */
static void ground_check(body_t *body) {
    int16_t bottom;
    uint8_t flags;
    if (body->vy < 0 || (body->state & BODY_ON_GROUND)) return;
    edges(body);
    bottom = E_BOTTOM + 1;                // first row below the feet
    if (bottom & 15) return;              // feet aren't on a tile boundary
    flags = probe_row(bottom, E_LEFT, E_RIGHT);
    if ((flags & TF_SOLID) || ((flags & TF_ONEWAY) && !(body->state & BODY_PASS_ONEWAY))) {
        body->y = PHYSICS_FP(bottom - body->h);
        body->vy = 0;
        body->state |= BODY_ON_GROUND;
    }
}

static uint8_t TOUCH_MASK = 0xFF;

void body_set_touch_mask(uint8_t mask) {
    TOUCH_MASK = mask;
}

static void touch(body_t *body) {
    int16_t cx, cy;
    uint8_t center;
    body->touched = 0;
    if (!TOUCH_MASK) return;
    edges(body);
    cx = E_LEFT + (body->w >> 1);
    cy = E_TOP + (body->h >> 1);
    // hazards use a slightly inset box so grazing a spike corner is forgiven
    center = probe_row(cy, cx, cx);
    body->touched = (center | probe_row(cy, E_LEFT + 2, E_RIGHT - 2) | probe_column(cx, E_TOP + 2, E_BOTTOM - 2)) & TOUCH_MASK;
    if (body->touched & TF_HAZARD) body->state |= BODY_IN_HAZARD;
    if (center & TOUCH_MASK & TF_LADDER) body->state |= BODY_ON_LADDER;
}

void body_step(body_t *body) {
    body->state &= BODY_PASS_ONEWAY;
    if (!tileflags_ready()) {
        // no tilemap/flags registered: move freely
        body->x += body->vx;
        body->y += body->vy;
        body->touched = 0;
        return;
    }
    move_x(body);
    move_y(body);
    ground_check(body);
    touch(body);
}
