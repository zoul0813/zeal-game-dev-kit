    .module controller_read16
    .globl _ctrl_read16
    .area _TEXT
; Shared single-DATA-line shifter; the caller chooses its timed clock routine.
; C=data mask, IX=clock+settling callback. Mouse callbacks also use D/E.
; HL=active-low inverted report, BC/AF clobbered (C preserved).
; First bit is available after LATCH; only the remaining 15 bits need CLOCK.
_ctrl_read16::
    ld hl,#0
    ld b,#16
read16_sample:
    in a,(0xd0)
    and c
    sub #1
    rr h
    rr l
    djnz read16_next_bit
    ret
read16_next_bit:
    call read16_clock
    jr read16_sample
read16_clock:
    jp (ix)
