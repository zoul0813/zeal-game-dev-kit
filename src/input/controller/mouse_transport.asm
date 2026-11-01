    .module controller_mouse_transport
    .globl _ctrl_mouse_byte, _ctrl_mouse_gap, _ctrl_mouse_start, _ctrl_mouse_header_clock, _ctrl_wait, _ctrl_mouse_clock
    .area _TEXT
; C=DATA mask, A=raw MSB-first mouse byte. Preserves C/DE/HL; clobbers AF/B.
; CLOCK rise->sample >=482 T. Inter-bit high phase preserves old C minimum.
_ctrl_mouse_byte::
    push de
    ld e,#0
    ld d,#8
mouse_byte_next_bit:
    nop
    call _ctrl_mouse_clock
    ld a,#31
    call _ctrl_wait
    nop
    in a,(0xd0)
    and c
    sub #1
    rl e
    dec d
    jr z,mouse_byte_done
    ld a,#15
    call _ctrl_wait
    jr mouse_byte_next_bit
mouse_byte_done:
    ld a,e
    pop de
    ret
; Shared inter-byte delay: callers preserve raw values with the stack.
_ctrl_mouse_gap::
    ld a,#74
    jp _ctrl_wait
; Four original delay calls plus header/byte-reader overhead. Shared by full
; polling and the extra detection drain; preserves BC/DE/HL.
_ctrl_mouse_start::
    ld a,#182
    jp _ctrl_wait
; Header timing remains separate from the pad's shorter bit loop. The
; shared shifter no longer pads every device with these legacy mouse holds.
; D=first hold count, E=subsequent hold count; C and E preserved.
_ctrl_mouse_header_clock::
    ld a,d
    ld d,e
    nop
    nop
    call _ctrl_wait
    call _ctrl_mouse_clock
    ex (sp),hl
    ex (sp),hl
    ex (sp),hl
    ex (sp),hl
    ret
