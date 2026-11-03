    .module physics_body_probe_column
    .globl _body_probe_column
    .globl __tf_height, __tf_width, __tf_outside, __tf_flags, _tileflags_rows
    .area _TEXT

; SDCC __sdcccall(1): fixed coordinate in HL, span start in DE, end on stack.
; Callee pops the stacked argument; returns flags in A.
; Clobbers AF/BC/DE/HL/IY; IX preserved.
; Requires a ready tileflags cache. Off-map pixels contribute __tf_outside.
_body_probe_column::
    ; hl = px, de = top, bottom on the stack (callee pops)
    pop     iy
    pop     bc
    push    iy
    ; the fixed coordinate must be on the map
    bit     7, h
    jp      NZ, column_outside
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
    jp      NZ, column_outside
    ld      a, l
    ld      hl, #__tf_width
    cp      a, (hl)
    jp      NC, column_outside
    push    af
    ; first tile in E, flags in D (outside when the span starts off-map)
    bit     7, d
    jr      Z, column_first_on_map
    ld      a, (__tf_outside)
    ld      d, a
    ld      e, #0
    jr      column_clamp_last
column_first_on_map:
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
    jr      Z, column_clamp_last
    ld      e, #0xFF
column_clamp_last:
    ; last tile in C, clamped to __tf_height - 1 (adds outside)
    bit     7, b
    jr      NZ, column_empty_span
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
    jr      NZ, column_last_outside
    ld      a, c
    cp      a, (hl)
    jr      C, column_count_tiles
column_last_outside:
    ld      a, (__tf_outside)
    or      a, d
    ld      d, a
    ld      a, (hl)
    or      a, a
    jr      Z, column_empty_span
    dec     a
    ld      c, a
column_count_tiles:
    ; C = tile count, none when first > last
    ld      a, c
    sub     a, e
    jr      C, column_empty_span
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
column_accumulate_flags:
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
    jr      NZ, column_accumulate_flags
    ld      a, b
    ret
column_empty_span:
    pop     af
    ld      a, d
    ret
column_outside:
    ld      a, (__tf_outside)
    ret
