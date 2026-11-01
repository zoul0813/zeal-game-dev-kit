    .module controller_init
    .globl _controller_init
    .area _TEXT
; SDCC: no arguments; A=ERR_SUCCESS. Clobbers AF only. 79 T.
_controller_init::
    ld a,#0xcf
    out (0xd2),a
    ld a,#3
    out (0xd2),a
    out (0xd2),a
    ld a,#8
    out (0xd0),a
    xor a
    ret
