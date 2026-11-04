    ; IX = sprite prefix. IY = permanent arena sprite; skip unused/hidden slots.
    ld l, 0 (ix)
    ld h, 1 (ix)
    push hl
    pop iy
    ld a, 2 (ix)
    inc a
    jp Z, 00980$
    ld a, 5 (ix)
    or a, a
    jp NZ, 00980$
