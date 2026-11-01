    .module controller_read16
    .globl _ctrl_read16, _ctrl_wait
    .area _TEXT
; Shared single-DATA-line transport. No IX/IY or interrupt changes.
; C=data mask, IX=clock routine, D=first high hold count, E=subsequent high hold count.
; Reads first bit without CLOCK, then 15 clocks. HL=active-low inverted report.
; B/D/AF clobbered; C/E preserved. Initial/sample settling is shared by both
; profiles; sensitivity adds its initial wait before entry.
_ctrl_read16::
    ld hl,#0
    ld b,#16
    jr read16_sample
read16_settle:
    ; Four stack exchanges restore HL/SP contents and spend 76 T in four
    ; instructions. Includes neither register spills nor a counted delay loop.
    ex (sp),hl
    ex (sp),hl
    ex (sp),hl
    ex (sp),hl
read16_sample:
    in a,(0xd0)
    and c
    sub #1
    rr h
    rr l
    djnz read16_next_bit
    ret
read16_next_bit:
    ld a,d
    ld d,e
    nop
    nop
    call _ctrl_wait
    call read16_clock
    jr read16_settle
read16_clock:
    jp (ix)
