    .module controller_set_mouse_sensitivity
    .globl _controller_set_mouse_sensitivity, _controller_get_mouse_sensitivity
    .globl _ctrl_wait
    .area _DATA
_ctrl_mouse_initialized:
    .ds 1
    .area _TEXT
; SDCC: A=port, L=sensitivity; A=achieved sensitivity or 0xFF.
; BC carries port mask/remaining attempts, DE carries port/target across calls.
; No IX/IY or interrupt changes. State is linked only with this setter.
_controller_set_mouse_sensitivity::
    cp #2
    jr nc,sensitivity_invalid
    ld h,a
    ld a,l
    cp #3
    jr nc,sensitivity_invalid
    ld d,h
    ld e,l
    ld c,h
    inc c
    push de
    push bc
    ld a,d
    call _controller_get_mouse_sensitivity
    pop bc
    pop de
    ld b,a
    ld a,(_ctrl_mouse_initialized)
    and c
    jr z,sensitivity_start_cycles
    ld a,b
    cp e
    ret z
sensitivity_start_cycles:
    ld b,#3
sensitivity_try_cycle:
    push bc
    push de
    ; Retain header-to-cycle recovery formerly supplied by C decoding.
    ld a,#36
    call _ctrl_wait
    call sensitivity_cycle
    ld a,d
    call _controller_get_mouse_sensitivity
    pop de
    pop bc
    cp e
    jr z,sensitivity_reached
    djnz sensitivity_try_cycle
sensitivity_invalid:
    ld a,#0xff
    ret
sensitivity_reached:
    ld a,(_ctrl_mouse_initialized)
    or c
    ld (_ctrl_mouse_initialized),a
    ld a,e
    ret
; Sensitivity cycle: high-LATCH clock pulse. Preserve old electrical minima.
sensitivity_cycle:
    ld a,#12
    out (0xd0),a
    ld a,#2
    call _ctrl_wait
    ld a,#4
    out (0xd0),a
    nop
    nop
    nop
    nop
    ld a,#12
    out (0xd0),a
    ld a,#2
    call _ctrl_wait
    ld a,#8
    out (0xd0),a
    ; Preserve cycle-to-next-report recovery, including old validation calls.
    ld a,#19
    jp _ctrl_wait
