    .module controller_latch
    .globl _ctrl_latch
    .area _TEXT
; Normal latch: 18 T high. AF clobbered, BC/DE/HL preserved.
_ctrl_latch::
    ld a,#12
    out (0xd0),a
    ld a,#8
    out (0xd0),a
    ret
