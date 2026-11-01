    .module controller_read_mouse
    .globl _controller_read_mouse, _ctrl_latch, _ctrl_read16, _ctrl_mouse_header_clock
    .globl _ctrl_mouse_byte, _ctrl_mouse_gap, _ctrl_mouse_start
    .area _TEXT
; SDCC: A=port, DE=ControllerMouse*. A=movement flags or 0xFF.
; Clobbers AF/BC/DE/HL; IX saved. No pad entry point or dual-port code.
_controller_read_mouse::
    cp #2
    jr nc,mouse_error
    ld c,a
    ld a,d
    or e
    jr z,mouse_error
    inc c
    push ix
    push de
    pop ix
    call _ctrl_latch
    push ix
    ld ix,#_ctrl_mouse_header_clock
    ld de,#0x0507
    call _ctrl_read16
    pop ix
    ld 0(ix),l
    ld 1(ix),h
    ; Header return + original four delay calls and byte-reader setup.
    call _ctrl_mouse_start
    call _ctrl_mouse_byte
    call mouse_decode_axis
    ld 3(ix),a
    call _ctrl_mouse_gap
    call _ctrl_mouse_byte
    call mouse_decode_axis
    ld 2(ix),a
    ld a,1(ix)
    and #0xf0
    cp #0x80
    jr nz,mouse_invalid_report
    ld a,3(ix)
    or a
    ld b,#0
    jr z,mouse_check_x
    inc b
mouse_check_x:
    ld a,2(ix)
    or a
    jr z,mouse_done
    set 1,b
mouse_done:
    ld a,b
    pop ix
    ret
mouse_invalid_report:
    xor a
    ld 0(ix),a
    ld 1(ix),a
    ld 2(ix),a
    ld 3(ix),a
    pop ix
mouse_error:
    ld a,#0xff
    ret
; Sign-magnitude conversion, including negative zero. Preserves BC/DE/HL.
mouse_decode_axis:
    bit 7,a
    res 7,a
    ret z
    neg
    ret
