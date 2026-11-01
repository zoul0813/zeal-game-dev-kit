    .module controller_get_mouse_sensitivity
    .globl _controller_get_mouse_sensitivity, _ctrl_sensitivity_header
    .area _TEXT
; SDCC: A=port; A=sensitivity or 0xFF. Preserves IX/IY.
_controller_get_mouse_sensitivity::
    cp #2
    jr nc,sensitivity_error
    call _ctrl_sensitivity_header
    ; Retain the original getter's MOUSE_ID bit check and speed encoding.
    bit 7,h
    jr z,sensitivity_error
    ld a,h
    and #12
    rrca
    rrca
    ; Header codes 0,2,1 correspond to low,medium,high.
    cp #3
    jr z,sensitivity_error
    or a
    ret z
    xor #3
    ret
sensitivity_error:
    ld a,#0xff
    ret
