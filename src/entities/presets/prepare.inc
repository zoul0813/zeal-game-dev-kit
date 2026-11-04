    ; IX = sprite prefix. IY = permanent arena sprite; skip unused/hidden slots.
    ld l, 0 (ix)
    ld h, 1 (ix)
    push hl
    pop iy
    ld a, 2 (ix)
    inc a
    jp Z, entity_park
    ld a, 5 (ix)
    or a, a
    jp NZ, entity_park
