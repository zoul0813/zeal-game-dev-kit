    .module controller_read_pads
    .globl _controller_read_pads, _ctrl_latch, _ctrl_wait, _ctrl_clock
    .area _TEXT
; SDCC: HL=output {port1,port2}. NULL performs no I/O.
; Clobbers AF/BC/DE/HL; IX saved. One IN per bit captures both DATA lines.
_controller_read_pads::
    ld a,h
    or l
    ret z
    push ix
    push hl
    pop ix
    xor a
    call _ctrl_latch
    ld hl,#0
    ld de,#0
    ld b,#16
pads_sample:
    ex (sp),hl
    ex (sp),hl
    ex (sp),hl
    ex (sp),hl
    in a,(0xd0)
    cpl
    rra
    rr h
    rr l
    rra
    rr d
    rr e
    djnz pads_next_bit
    ld 0(ix),l
    ld 1(ix),h
    ld 2(ix),e
    ld 3(ix),d
    pop ix
    ret
pads_next_bit:
    ld a,#9
    call _ctrl_wait
    call _ctrl_clock
    jr pads_sample
