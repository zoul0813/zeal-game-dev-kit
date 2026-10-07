; Runtime type dispatch deliberately retains mouse reads and their side effects.
; call(1): port in A, ControllerType in L, boolean result in A.
.module snes_controller_is
.globl _controller_is, _snes_identify, _zgdk_controller_is_pad
.globl _zgdk_controller_is_runtime
.globl _controller_read_mouse
.area _TEXT
_controller_is::
_zgdk_controller_is_runtime::
    ld c, a
    ld a, l
    or a
    ld a, c
    jp z, _zgdk_controller_is_pad
    push hl
    call _snes_identify
    pop hl
    ld a, l
    dec a
    jr nz, false$
    push de
    push bc
    ld a, c
    call _controller_read_mouse
    pop bc
    ld a, c
    call _controller_read_mouse
    pop de
    ld a, d
    and #0xf0
    cp #0x80
    ld a, #0
    ret nz
    inc a
    ret
false$:
    xor a
    ret
