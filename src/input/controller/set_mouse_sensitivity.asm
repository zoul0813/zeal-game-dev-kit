; call(1): port in A, sensitivity in L, result in A (0xff on failure).
; No cache/movement side effects. A first successful set must cycle even when
; already at target. SNS-016 cycles low/medium/high; Hyperkin may ignore cycles.
.module snes_mouse_sensitivity
.globl _controller_get_mouse_sensitivity, _controller_set_mouse_sensitivity
.globl _snes_read16
.area _DATA
initialized$: .ds 1
.area _TEXT
_controller_get_mouse_sensitivity::
    cp #2
    jr nc, error$
    push af
    call _snes_read16
    pop af
    or a
    jr nz, selected$
    ex de, hl
selected$:
    bit 7, d             ; preserve existing permissive MOUSE_ID mask test
    jr z, error$
    ld a, d
    and #0x0c
    ret z                ; low
    cp #8
    ld a, #1
    ret z                ; medium = BUTTON_R
    ld a, d
    and #0x0c
    cp #4
    ld a, #2
    ret z                ; high = BUTTON_L
error$:
    ld a, #0xff
    ret
_controller_set_mouse_sensitivity::
    cp #2
    jr nc, error$
    ld h, a              ; keep port/target in HL across getter
    ld a, l
    cp #3
    jr nc, error$
    ld c, #1
    bit 0, h
    jr z, mask_done$
    inc c
mask_done$:
    push hl
    push bc
    ld a, h
    call _controller_get_mouse_sensitivity
    pop bc
    pop hl
    ld e, a
    ld a, (initialized$)
    and c
    jr z, start$
    ld a, e
    cp l
    ret z
start$:
    ld b, #3
retry$:
    call cycle$          ; only clobbers A
    push hl
    push bc
    ld a, h
    call _controller_get_mouse_sensitivity
    pop bc
    pop hl
    cp l
    jr z, success$
    djnz retry$
    jr error$
success$:
    ld a, (initialized$)
    or c
    ld (initialized$), a
    ld a, l
    ret
cycle$:
    ld a, #12
    out (0xd0), a
    call report_delay$
    ld a, #4
    out (0xd0), a
    call report_delay$
    ld a, #12
    out (0xd0), a
    call report_delay$
    ld a, #8
    out (0xd0), a
    jp report_delay$
report_delay$:
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    ret                  ; same 32 NOP T as the old report delay, plus call/ret
