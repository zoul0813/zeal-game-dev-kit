    .module controller_mouse_clock
    .globl _ctrl_mouse_clock
    .area _TEXT
; Mouse clock: 18 T low; callbacks enforce Hyperkin read-to-read delays. AF clobbered, BC/DE/HL preserved.
_ctrl_mouse_clock::
    xor a
    out (0xd0),a
    ld a,#8
    out (0xd0),a
    ret
