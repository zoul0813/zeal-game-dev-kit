    .module controller_clock
    .globl _ctrl_clock
    .area _TEXT
; Normal clock: 18 T low. AF clobbered, BC/DE/HL preserved.
_ctrl_clock::
    xor a
    out (0xd0),a
    ld a,#8
    out (0xd0),a
    ret
