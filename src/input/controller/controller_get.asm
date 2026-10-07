.module snes_controller_get
.globl _controller_get, _PORT1_bits, _PORT2_bits
.area _TEXT
_controller_get::
    or a
    jr z, port1$
    dec a
    ld de, #0
    ret nz
    ld de, (_PORT2_bits)
    ret
port1$:
    ld de, (_PORT1_bits)
    ret
