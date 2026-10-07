; Zeroed by the SDK CRT. Keeping mouse coordinates here lets flush retain its
; public side effects without linking any mouse protocol code.
.module snes_state
.globl _PORT1_bits, _PORT2_bits, _MOUSE_y, _MOUSE_x
.area _DATA
_PORT1_bits:: .ds 2
_PORT2_bits:: .ds 2
_MOUSE_y:: .ds 1
_MOUSE_x:: .ds 1
