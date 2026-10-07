; Separate archive member for constant SNES_PAD detection in the public header.
.module snes_controller_is_pad
.globl _zgdk_controller_is_pad, _snes_identify
.globl _controller_flush, _controller_read_port
.area _TEXT
_zgdk_controller_is_pad::
    call _snes_identify
    ld a, d
    and #0xf0
    ld a, #0
    ret nz
    inc a
    ret
; Shared detection preamble; return selected report in DE and port in C.
_snes_identify::
    push af
    call _controller_flush
    pop af
    push af
    call _controller_read_port
    pop af
    ld c, a
    ret
