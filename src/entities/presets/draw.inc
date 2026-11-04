    ; sx = ((int16)(x + off_x) >> 4) + ox   (screen x + 16)
    ; Keep the sign: a positive offset can bring a negative origin into view.
    .ifeq ENTITY_BODY
    push    de
    .endif
    ld      bc, (__entity_view_off_x)
    add     hl, bc
    sra     h
    rr      l
    sra     h
    rr      l
    sra     h
    rr      l
    sra     h
    rr      l
    ld      a, 6 (ix)
    ld      c, a
    rla
    sbc     a, a
    ld      b, a
    add     hl, bc
    ; visible when sx - 1 < 335
    ex      de, hl
    ld      l, e
    ld      h, d
    dec     hl
    ld      bc, (__entity_view_limit_x)
    or      a, a
    sbc     hl, bc
    jp      NC, entity_cull_x
    ; sy = ((int16)(y + off_y) >> 4) + oy   (screen y + 32)
    .ifne ENTITY_BODY
    ld      l, 11 (ix)
    ld      h, 12 (ix)
    .else
    pop     hl
    .endif
    ld      bc, (__entity_view_off_y)
    add     hl, bc
    sra     h
    rr      l
    sra     h
    rr      l
    sra     h
    rr      l
    sra     h
    rr      l
    ld      a, 7 (ix)
    ld      c, a
    rla
    sbc     a, a
    ld      b, a
    add     hl, bc
    ; Hardware Y is unsigned screen y + 16. A tall sprite at Y=0
    ; still shows its bottom tile; coordinates above screen y=-16
    ; cannot be represented. Normal sprites need Y>=1, tall need Y>=0.
    ld      bc, #-17
    add     hl, bc
    ld      bc, (__entity_view_limit_y)
    ld      a, 4 (ix)
    or      a, a
    jr      Z, entity_check_y
    inc     hl
    inc     bc
entity_check_y:
    ld      a, l
    sub     a, c
    ld      a, h
    sbc     a, b
    jp      NC, entity_park
    ; Restore hardware Y after the height-dependent visibility check.
    ld      a, 4 (ix)
    or      a, a
    jr      NZ, entity_write_position
    inc     hl
entity_write_position:
    ld      0 (iy), l
    ld      1 (iy), h
    ld      2 (iy), e
    ld      3 (iy), d
    ld      8 (ix), #0
    ; sprite tile is retained from anim_tick (or seeded by play/restart)
    ; flags = (flags & 0xF3) | (flip & 0x0C)
    ld      a, 3 (ix)
    and     a, #0x0C
    ld      c, a
    ld      a, 5 (iy)
    and     a, #0xF3
    or      a, c
    ld      5 (iy), a
    ; options = tall ? SPRITE_OPTION_32PX : 0
    ld      a, 4 (ix)
    or      a, a
    jr      Z, entity_write_options
    ld      a, #2
entity_write_options:
    ld      6 (iy), a
    ld      7 (iy), #0
    jp      entity_sync_done
entity_cull_x:
    .ifeq ENTITY_BODY
    pop     hl
    .endif
entity_park:
    ; park once: y = 0 and options = 0 (a 16x32 sprite at y = 0 would show its bottom half)
    ld      a, 8 (ix)
    or      a, a
    jr      NZ, entity_sync_done
    ld      8 (ix), #1
    xor     a, a
    ld      0 (iy), a
    ld      1 (iy), a
    ld      6 (iy), a
    ld      7 (iy), a
