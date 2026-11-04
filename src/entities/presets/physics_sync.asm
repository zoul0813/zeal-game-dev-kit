.module entity_physics_sync
.globl _physics_entity_sync
.globl __entity_view_off_x, __entity_view_off_y
.globl __entity_view_limit_x, __entity_view_limit_y
ENTITY_BODY = 1
.area _TEXT
_physics_entity_sync::
    push ix
    push iy
    push hl
    pop ix
.include "prepare.inc"
    ld l, 9 (ix)
    ld h, 10 (ix)
.include "draw.inc"
00990$:
    pop iy
    pop ix
    ret
