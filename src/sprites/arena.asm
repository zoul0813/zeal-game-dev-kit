; Sprite arena API, SDCC Z80 call(1). gfx_sprite is exactly eight bytes.
.module sprite_arena
.optsdcc -mz80
.globl _sprites_register_arena, _sprites_register, _sprites_deregister
.globl _zgdk_sprite_detect_dma
.globl _zgdk_sprite_arena, _zgdk_sprite_size, _zgdk_sprite_count, _zgdk_sprite_rendered, _zgdk_sprite_dma
.area _DATA
_zgdk_sprite_arena:: .ds 2
_zgdk_sprite_size:: .ds 1
_zgdk_sprite_count:: .ds 1
_zgdk_sprite_rendered:: .ds 1
_zgdk_sprite_dma:: .ds 1
.area _TEXT
; HL = arena; size is a one-byte stack argument, callee removes it.
_sprites_register_arena::
    ex de, hl
    pop bc
    ld hl, #0
    add hl, sp
    ld a, (hl)
    inc sp
    push bc
    or a
    jr z, invalid$
    cp #129
    jr nc, invalid$
    ld c, a
    ld a, d
    or e
    jr z, invalid$
    ld (_zgdk_sprite_arena), de
    ld a, c
    ld (_zgdk_sprite_size), a
    xor a
    ld (_zgdk_sprite_count), a
    ; A one-byte private RAM transfer probes DMA once per arena init.
    call _zgdk_sprite_detect_dma
    xor a
    ret

invalid$:
    ld a, #2
    ret

; Eight-byte by-value argument on the stack; DE = returned slot or NULL.
_sprites_register::
    ld de, (_zgdk_sprite_arena)
    ld a, d
    or e
    jr z, full$
    ld a, (_zgdk_sprite_size)
    ld c, a
    ld a, (_zgdk_sprite_count)
    cp c
    jr nc, full$
    ld l, a
    ld h, #0
    add hl, hl
    add hl, hl
    add hl, hl
    add hl, de
    ex de, hl
    ld hl, #2
    add hl, sp
    push de
    ld bc, #8
    ldir
    pop de
    ld hl, #_zgdk_sprite_count
    inc (hl)
    jr register_return$
full$:
    ld de, #0
register_return$:
    pop hl
    pop bc
    pop bc
    pop bc
    pop bc
    jp (hl)

_sprites_deregister::
    xor a
    ld (_zgdk_sprite_count), a
    ld hl, (_zgdk_sprite_arena)
    ld a, h
    or l
    ret z
    push hl
    ld a, (_zgdk_sprite_size)
    ld l, a
    ld h, #0
    add hl, hl
    add hl, hl
    add hl, hl
    ld c, l
    ld b, h
    pop hl
    ld (hl), #0
    ld e, l
    ld d, h
    inc de
    dec bc
    ldir
    ret
