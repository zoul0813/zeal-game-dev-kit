; Mouse movement starts AFTER controller_read_port's 16 status bits; no latch.
; Coordinates are MSB-first sign/magnitude, Y then X. call(1): port/result in A.
.module snes_mouse
.globl _controller_read_mouse, _MOUSE_y, _MOUSE_x
.area _TEXT
_controller_read_mouse::
    ld e, #1
    or a
    jr z, mask_done$
mask$:
    sla e
    dec a
    jr nz, mask$
mask_done$:
    ld b, #144
    call wait$           ; 1901 T incl. LD/CALL, >= four old bit delays (1884 T)
    call byte$
    call decode$
    ld (_MOUSE_y), a
    call delay$
    call byte$
    call decode$
    ld (_MOUSE_x), a
    ld c, #0
    or a
    jr z, no_x$
    ld c, #2
no_x$:
    ld a, (_MOUSE_y)
    or a
    ld a, c
    ret z
    or #1
    ret
byte$:
    ld c, #8
    ld d, #0
bit$:
    xor a
    out (0xd0), a
    ld b, #4
low_wait$:
    djnz low_wait$
    ld a, #8
    out (0xd0), a         ; clock low >= 7.2 us
    call delay$          ; preserve clone-compatible settling time
    in a, (0xd0)
    and e
    sub #1               ; carry = pressed (active low)
    rl d
    dec c
    jr nz, bit$
    ld a, d
    ret
decode$:
    ld d, a
    and #0x7f
    bit 7, d
    ret z
    neg                  ; includes negative zero -> zero
    ret
delay$:
    ld b, #34
wait$:
    djnz wait$
    ret                  ; CALL + delay = 471 T, same as old SDCC NOP_WAIT(16)
