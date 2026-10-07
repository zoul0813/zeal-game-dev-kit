.module sprite_point
.optsdcc -mz80
.globl _sprite_get_point, _sprite_set_point, _sprite_set_xy
.area _TEXT
; HL = sprite (y,x,tile,flags,options); DE = Point (x,y).
_sprite_get_point::
    push hl
    inc hl
    inc hl
    ldi
    ldi
    pop hl
    ldi
    ldi
    ret
_sprite_set_point::
    ex de, hl
    push de
    inc de
    inc de
    ldi
    ldi
    pop de
    ldi
    ldi
    ret
; HL = sprite; DE = x; y is two-byte stack argument, callee removes it.
_sprite_set_xy::
    ld c, l
    ld b, h
    inc hl
    inc hl
    ld (hl), e
    inc hl
    ld (hl), d
    pop hl
    pop de
    ld a, e
    ld (bc), a
    inc bc
    ld a, d
    ld (bc), a
    jp (hl)
