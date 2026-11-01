    .module controller_latch
    .globl _ctrl_latch, _ctrl_wait
    .area _TEXT
; 124 T high (12.4 us at 10 MHz), meeting the 12 us console latch target.
; AF clobbered; BC/DE/HL preserved.
_ctrl_latch::
    ld a,#12
    out (0xd0),a
    ld a,#4
    call _ctrl_wait
    ld a,#8
    out (0xd0),a
    ret
