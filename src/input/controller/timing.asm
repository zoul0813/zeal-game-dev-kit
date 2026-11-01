    .module controller_timing
    .globl _ctrl_wait
    .area _TEXT
; Wait A=1..255: 13*A+30 T excluding CALL. Preserves BC/DE/HL.
_ctrl_wait::
    push bc
    ld b,a
wait_loop:
    djnz wait_loop
    pop bc
    ret
