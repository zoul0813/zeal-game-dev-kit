.module entity_physics_pool_sync
.globl _physics_entities_sync
.globl __entity_view_off_x, __entity_view_off_y
.globl __entity_view_limit_x, __entity_view_limit_y
.area _DATA
_sync_end: .ds 2
_sync_stride: .ds 1
ENTITY_BODY = 1
.area _TEXT
_physics_entities_sync::
    ld a, h
    or a, l
    ret Z
    push ix
    push iy
    push hl
    pop iy
    ld a, 5 (iy)
    cp #21
    jp C, pool_sync_return
    ld (_sync_stride), a
    ld a, 6 (iy)
    cp #2
    jp NZ, pool_sync_return
    ld l, 0 (iy)
    ld h, 1 (iy)
    ld a, h
    or a, l
    jp Z, pool_sync_return
    push hl
    pop ix
    ld l, 2 (iy)
    ld h, 3 (iy)
    ld (_sync_end), hl
pool_sync_loop:
    push ix
    pop hl
    ld de, (_sync_end)
    or a, a
    sbc hl, de
    jp NC, pool_sync_return
.include "prepare.inc"
    ld l, 9 (ix)
    ld h, 10 (ix)
.include "draw.inc"
entity_sync_done:
    ld a, (_sync_stride)
    ld c, a
    ld b, #0
    add ix, bc
    jp pool_sync_loop
pool_sync_return:
    pop iy
    pop ix
    ret
