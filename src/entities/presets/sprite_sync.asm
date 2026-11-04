.module entity_sprite_sync
.globl _sprite_entity_sync_fp
.globl __entity_view_off_x, __entity_view_off_y
.globl __entity_view_limit_x, __entity_view_limit_y
ENTITY_BODY = 0
.area _TEXT
_sprite_entity_sync_fp::
    push ix
    push iy
    push hl
    pop ix
.include "prepare.inc"
    ; First position is DE, second is on the caller's stack.
    ex de, hl
    push hl
    ld hl, #8
    add hl, sp
    ld e, (hl)
    inc hl
    ld d, (hl)
    pop hl
.include "draw.inc"
entity_sync_done:
    pop iy
    pop ix
    pop hl
    pop bc
    jp (hl)
