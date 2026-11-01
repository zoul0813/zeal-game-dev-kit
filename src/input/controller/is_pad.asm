    .module controller_is_pad
    .globl _controller_is_pad, _controller_read_pad
    .area _TEXT
; SDCC: A=port; A=boolean. Raw invalid-port report cannot identify a pad.
_controller_is_pad::
    call _controller_read_pad
    ld a,d
    and #0xf0
    sub #1
    sbc a,a
    neg
    ret
