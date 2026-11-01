    .module controller_pad_clock
    .globl _ctrl_pad_clock
    .area _TEXT
; Pad timing at 10 MHz, following Jim Christy's measured console waveform.
; CLOCK low = 60 T (6 us); return/loop/IN provide >=60 T of high settling.
; AF clobbered; BC/DE/HL preserved. No mouse timing helpers are referenced.
_ctrl_pad_clock::
    xor a
    out (0xd0),a
    ; 42 T of low hold, in addition to LD A / OUT's 18 T.
    ex (sp),hl
    ex (sp),hl
    nop
    ld a,#8
    out (0xd0),a
    ; 28 T plus RET (10), caller JR (12), IN (11) = 61 T to sampling.
    push af
    pop af
    ld a,#8
    ret
