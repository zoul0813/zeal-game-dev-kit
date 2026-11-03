    .module physics_body_probe_row
    .globl _body_probe_row
    .globl __tf_height, __tf_width, __tf_outside, __tf_flags, _tileflags_rows
    .area _TEXT

; SDCC __sdcccall(1): fixed coordinate in HL, span start in DE, end on stack.
; Callee pops the stacked argument; returns flags in A.
; Clobbers AF/BC/DE/HL/IY; IX preserved.
; Requires a ready tileflags cache. Off-map pixels contribute __tf_outside.
_body_probe_row::
    ; hl = py, de = left, right on the stack (callee pops)
    pop     iy
    pop     bc
    push    iy
    ; the fixed coordinate must be on the map
    bit     7, h
    jp      NZ, row_outside
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
    jp      NZ, row_outside
    ld      a, l
    ld      hl, #__tf_height
    cp      a, (hl)
    jp      NC, row_outside
    push    af
    ; first tile in E, flags in D (outside when the span starts off-map)
    bit     7, d
    jr      Z, row_first_on_map
    ld      a, (__tf_outside)
    ld      d, a
    ld      e, #0
    jr      row_clamp_last
row_first_on_map:
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
    jr      Z, row_clamp_last
    ld      e, #0xFF
row_clamp_last:
    ; last tile in C, clamped to __tf_width - 1 (adds outside)
    bit     7, b
    jr      NZ, row_empty_span
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
    jr      NZ, row_last_outside
    ld      a, c
    cp      a, (hl)
    jr      C, row_count_tiles
row_last_outside:
    ld      a, (__tf_outside)
    or      a, d
    ld      d, a
    ld      a, (hl)
    or      a, a
    jr      Z, row_empty_span
    dec     a
    ld      c, a
row_count_tiles:
    ; C = tile count, none when first > last
    ld      a, c
    sub     a, e
    jr      C, row_empty_span
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
row_accumulate_flags:
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
    jr      NZ, row_accumulate_flags
    ld      a, b
    ret
row_empty_span:
    pop     af
    ld      a, d
    ret
row_outside:
    ld      a, (__tf_outside)
    ret
