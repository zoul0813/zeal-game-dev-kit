    .module controller_sensitivity_header
    .globl _ctrl_sensitivity_header, _ctrl_read16, _ctrl_wait
    .area _TEXT
; A=valid port; HL=header. Shared transport, longer sensitivity timing profile.
_ctrl_sensitivity_header::
    inc a
    ld c,a
    ld a,#12
    out (0xd0),a
    ld a,#5
    call _ctrl_wait
    ld a,#8
    out (0xd0),a
    ld a,#8
    call _ctrl_wait
    push ix
    ld ix,#sensitivity_clock
    ld de,#0x1817
    call _ctrl_read16
    pop ix
    ret
; The slow clock belongs only to sensitivity support; polling cannot link it.
sensitivity_clock:
    xor a
    out (0xd0),a
    ld a,#1
    call _ctrl_wait
    ld a,#8
    out (0xd0),a
    ret
