; Common first 16 bits: pad buttons / mouse status. Active-low data, LSB first.
; PIO A: DATA1=0, DATA2=1, LATCH=2, CLOCK=3. Timing assumes a 10 MHz Z80.
; Internal ABI: returns port 1 in HL, port 2 in DE; clobbers AF/BC.
; Does not touch public caches, IX/IY, alternate registers or interrupt state.
.module snes_serial
.globl _snes_read16
.area _TEXT
_snes_read16::
    ld a, #12
    out (0xd0), a
    ld b, #8
latch_wait$:
    djnz latch_wait$
    ld a, #8
    out (0xd0), a         ; latch high: 124 T = 12.4 us
    ld b, #2
first_wait$:
    djnz first_wait$
    ld hl, #0
    ld de, #0
    ld c, #16            ; first sample needs no clock edge
sample$:
    in a, (0xd0)
    cpl
    rra
    rr h
    rr l
    rra
    rr d
    rr e                 ; one input read captures both ports
    dec c
    ret z                ; leave clock high, exactly 15 pulses
    xor a
    out (0xd0), a
    ld b, #4
low_wait$:
    djnz low_wait$
    ld a, #8
    out (0xd0), a         ; clock low: 72 T = 7.2 us
    ld b, #4
high_wait$:
    djnz high_wait$
    jr sample$           ; rising edge to sample: 77 T = 7.7 us
