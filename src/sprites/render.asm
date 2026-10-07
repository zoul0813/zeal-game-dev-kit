; Synchronous sprite upload, preserving MMU page 0, peripheral bank and IFF2.
; Arena initialization probes DMA once; unsupported boards use LDIR.
; Code, stack and SDK application data must reside outside MMU page 0,
; as required by the SDK's VRAM mapping convention. The source arena may
; also be in page 0: the fallback reads it before temporarily mapping VRAM.
.module sprite_render
.optsdcc -mz80
.globl _sprites_render, _zgdk_sprite_dma_render
.globl _zgdk_sprite_arena, _zgdk_sprite_count, _zgdk_sprite_rendered, _zgdk_sprite_dma
.area _DATA
saved_page: .ds 1
vram_page: .ds 1
.area _TEXT
_sprites_render::
    ld a, h
    or l
    jp z, invalid$
    ld hl, (_zgdk_sprite_arena)
    ld a, h
    or l
    jp z, invalid$
    ld a, (_zgdk_sprite_count)
    ld b, a
    ld a, (_zgdk_sprite_rendered)
    cp b
    jr nc, maximum$
    ld a, b
maximum$:
    or a
    ret z
    ld c, a
    push ix
    ld a, i
    di
    push af
    ; DMA setup outweighs LDIR for small updates. Eight slots = 64 bytes.
    ld a, c
    cp #8
    jr c, fallback$
    ld a, (_zgdk_sprite_dma)
    or a
    jr z, fallback$
    call _zgdk_sprite_dma_render
    jr finished$
fallback$:
    xor a
    in a, (#0xf0)
    ld (saved_page), a
    in a, (#0x8f)
    and #0x1f
    rlca
    rlca
    rlca
    ld (vram_page), a
    ld hl, (_zgdk_sprite_arena)
    ld a, (_zgdk_sprite_count)
    call byte_count$
    ld de, #0x2800
    ld a, b
    or c
    jr z, clear_old$
    ; Page-zero source bytes would disappear under the VRAM mapping.
    ld a, h
    cp #0x40
    jr c, low_source$
    ld a, (vram_page)
    out (#0xf0), a
    ldir
    jr clear_old$
low_source$:
    ; Rare ROM/page-zero arena: bounce each byte through A. Once past
    ; page zero the remaining contiguous source uses the fast block copy.
    ld a, (saved_page)
    out (#0xf0), a
    ld a, (hl)
    push af
    ld a, (vram_page)
    out (#0xf0), a
    pop af
    ld (de), a
    inc hl
    inc de
    dec bc
    ld a, b
    or c
    jr z, clear_old$
    ld a, h
    cp #0x40
    jr c, low_source$
    ldir
clear_old$:
    ld a, (vram_page)
    out (#0xf0), a
    ld a, (_zgdk_sprite_count)
    ld c, a
    ld a, (_zgdk_sprite_rendered)
    sub c
    jr c, restore_page$
    jr z, restore_page$
    call byte_count$
    ; DE is the first stale slot. Seed zero, then propagate with LDIR.
    ld h, d
    ld l, e
    ld (hl), #0
    inc de
    dec bc
    ldir
restore_page$:
    ld a, (saved_page)
    out (#0xf0), a
finished$:
    ld a, (_zgdk_sprite_count)
    ld (_zgdk_sprite_rendered), a
    pop af
    pop ix
    jp po, disabled$
    xor a
    ei
    ret
disabled$:
    xor a
    ret
invalid$:
    ld a, #2
    ret

; A sprite count -> BC byte count, preserving HL and DE.
byte_count$:
    push hl
    ld l, a
    ld h, #0
    add hl, hl
    add hl, hl
    add hl, hl
    ld c, l
    ld b, h
    pop hl
    ret
