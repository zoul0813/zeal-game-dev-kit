    .module controller_read_pad
    .globl _controller_read_pad, _ctrl_latch, _ctrl_read16, _ctrl_pad_clock
    .area _TEXT
; SDCC: A=port; DE=raw result. Invalid port: 0xFFFF without I/O.
; Clobbers AF/BC/DE/HL. Shared reader has no second-port accumulation.
_controller_read_pad::
    cp #2
    jr nc,pad_invalid_port
    push ix
    inc a
    ld c,a
    call _ctrl_latch
    ld ix,#_ctrl_pad_clock
    call _ctrl_read16
    ex de,hl
    pop ix
    ret
pad_invalid_port:
    ld de,#0xffff
    ret
