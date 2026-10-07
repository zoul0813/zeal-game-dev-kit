; SDCC Z80 call(1): uint8/enum argument and result in A, uint16 result in DE.
.module snes_controller
.globl _controller_init, _controller_flush, _controller_read_port
.globl _controller_get, _snes_read16
.globl _PORT1_bits, _PORT2_bits, _MOUSE_y, _MOUSE_x
.area _TEXT
_controller_init::
    ld a, #0xcf
    out (0xd2), a
    ld a, #3             ; only the two data pins are inputs
    out (0xd2), a
    out (0xd2), a        ; disable PIO interrupts (0x03)
    ld a, #8
    out (0xd0), a
    xor a
    ret
_controller_flush::
    ld hl, #0
    ld (_PORT1_bits), hl
    ld (_PORT2_bits), hl
    xor a
    ld (_MOUSE_y), a
    ld (_MOUSE_x), a
    ret
_controller_read_port::
    push af
    call _snes_read16
    ld (_PORT1_bits), hl
    ld (_PORT2_bits), de
    pop af
    jp _controller_get   ; invalid ports still refresh both caches, return zero
