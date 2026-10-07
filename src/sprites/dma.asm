; DMA capability belongs to the transfer backend, not a firmware version.
; Uses documented descriptor registers and the hardware's BUSREQ handshake.
.module sprite_dma
.optsdcc -mz80
.globl _zgdk_sprite_detect_dma, _zgdk_sprite_dma_render
.globl _zgdk_sprite_arena, _zgdk_sprite_count, _zgdk_sprite_rendered, _zgdk_sprite_dma
.area _DATA
; One of these two descriptor positions fits wholly within a 16KB MMU page.
descriptor_buffer: .ds 24
zero_byte: .ds 1
probe_source: .ds 1
probe_result: .ds 1
.area _TEXT
; Test a one-byte copy between private RAM bytes, without touching VRAM.
; On a missing controller the start write is ignored and result stays zero.
; Keep the default/user-configured DMA clock; preserve IFF2 and I/O bank.
_zgdk_sprite_detect_dma::
    push ix
    ld a, i
    di
    push af
    in a, (#0x8e)
    push af
    ld a, #4
    out (#0x8e), a
    call descriptor_init$
    ld a, #0xa5
    ld (probe_source), a
    xor a
    ld (probe_result), a
    ld (zero_byte), a
    ld hl, #probe_source
    call physical$
    ld 0(ix), e
    ld 1(ix), d
    ld 2(ix), c
    ld hl, #probe_result
    call physical$
    ld 3(ix), e
    ld 4(ix), d
    ld 5(ix), c
    ld 6(ix), #1
    ld 7(ix), #0
    ld 8(ix), #1
    call submit$
    ld a, (probe_result)
    cp #0xa5
    ld a, #0
    jr nz, probe_done$
    inc a
probe_done$:
    ld (_zgdk_sprite_dma), a
    pop af
    out (#0x8e), a
    pop af
    pop ix
    ret po
    ei
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

; HL virtual -> CDE physical, using the currently mapped MMU page.
; Zeal decodes the high two I/O address bits to select an MMU register.
physical$:
    ld e, l
    ld a, h
    and #0x3f
    ld d, a
    ld a, h
    in a, (#0xf0)
    rrca
    rrca
    ld c, a
    and #0xc0
    or d
    ld d, a
    ld a, c
    and #0x3f
    ld c, a
    ret

_zgdk_sprite_dma_render::
    in a, (#0x8e)
    push af
    ld a, #4
    out (#0x8e), a
    call descriptor_init$
    in a, (#0x8f)
    and #0x1f
    add a, a
    ld 5(ix), a
    ld hl, (_zgdk_sprite_arena)
    ld a, (_zgdk_sprite_count)
    call byte_count$
    ld de, #0x2800
    ld a, #1                  ; last, increment read and write
    call dma_copy$
    ld a, (_zgdk_sprite_count)
    ld c, a
    ld a, (_zgdk_sprite_rendered)
    sub c
    jr c, dma_done$
    jr z, dma_done$
    call byte_count$
    ld e, 3(ix)
    ld d, 4(ix)
    ld hl, #zero_byte
    ld a, #5                  ; last, fixed read, increment write
    call dma_copy$
dma_done$:
    pop af
    out (#0x8e), a
    ret

descriptor_init$:
    ld hl, #descriptor_buffer
    ld a, h
    and #0x3f
    cp #0x3f
    jr nz, descriptor_ready$
    ld a, l
    cp #0xf5
    jr c, descriptor_ready$
    ld de, #12
    add hl, de
descriptor_ready$:
    push hl
    pop ix
    ; Initialize all reserved bytes; hardware currently ignores these.
    xor a
    ld 9(ix), a
    ld 10(ix), a
    ld 11(ix), a
    ret

; HL = virtual source, DE = VRAM offset, BC = size, A = descriptor flags.
; Splitting each source span at 16KB boundaries handles discontiguous MMU
; pages. Fixed-source zero fills need only one descriptor submission.
dma_copy$:
    ld 3(ix), e
    ld 4(ix), d
    ld 8(ix), a
    ld a, b
    or c
    ret z
dma_chunk$:
    push bc
    push hl
    call physical$
    ld 0(ix), e
    ld 1(ix), d
    ld 2(ix), c
    pop hl
    pop bc
    bit 2, 8(ix)
    jr nz, whole_chunk$
    ; DE = bytes until next source MMU page.
    push hl
    ld a, h
    and #0x3f
    ld h, a
    ex de, hl
    ld hl, #0x4000
    or a
    sbc hl, de
    ex de, hl
    pop hl
    ld a, b
    cp d
    jr c, whole_chunk$
    jr nz, chunk_ready$
    ld a, c
    cp e
    jr nc, chunk_ready$
whole_chunk$:
    ld e, c
    ld d, b
chunk_ready$:
    ld 6(ix), e
    ld 7(ix), d
    add hl, de
    ld a, c
    sub e
    ld c, a
    ld a, b
    sbc a, d
    ld b, a
    push hl
    push bc
    call submit$
    ld e, 6(ix)
    ld d, 7(ix)
    ld l, 3(ix)
    ld h, 4(ix)
    add hl, de
    ld 3(ix), l
    ld 4(ix), h
    pop bc
    pop hl
    ld a, b
    or c
    jp nz, dma_chunk$
    ret

submit$:
    push ix
    pop hl
    call physical$
    ld a, e
    out (#0xa1), a
    ld a, d
    out (#0xa2), a
    ld a, c
    out (#0xa3), a
    ld a, #0x80
    out (#0xa0), a
    ; BUSREQ stalls the Z80 until this descriptor completes. Do not poll:
    ; the real 1.0 DMA controller has no readable busy/status register.
    nop
    ret
