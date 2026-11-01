    .module controller_is_mouse
    .globl _controller_is_mouse, _controller_read_mouse
    .globl _ctrl_mouse_byte, _ctrl_mouse_gap, _ctrl_mouse_start, _ctrl_wait
    .area _TEXT
; SDCC: A=port; A=boolean. Uses a four-byte stack report, no static state.
; Preserve the legacy 48-bit detection transaction, including trailing gap.
_controller_is_mouse::
    cp #2
    jr nc,mouse_not_present
    push af
    ld hl,#-4
    add hl,sp
    ld sp,hl
    ex de,hl
    call _controller_read_mouse
    ld hl,#4
    add hl,sp
    ld sp,hl
    ld b,a
    pop af
    inc a
    ld c,a
    push bc
    ; Include the shortest invalid-report return path in the recovery bound.
    ld a,#2
    call _ctrl_wait
    call _ctrl_mouse_start
    call _ctrl_mouse_byte
    ; Discarding skips axis decoding; retain that inter-byte settling time.
    ld a,#1
    call _ctrl_wait
    call _ctrl_mouse_gap
    call _ctrl_mouse_byte
    pop bc
    ld a,b
    cp #0xff
    ld a,#0
    ret z
    inc a
    ret
mouse_not_present:
    xor a
    ret
